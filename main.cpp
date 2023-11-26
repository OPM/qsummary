/*
   Copyright 2022 Equinor ASA.

   This file is part of the Open Porous Media project (OPM).

   OPM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   OPM is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with OPM.  If not, see <http://www.gnu.org/licenses/>.
   */

#include <QtWidgets/QApplication>

#include <appl/smry_appl.hpp>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <getopt.h>
#include <algorithm>
#include <type_traits>

#include <opm/io/eclipse/ESmry.hpp>
#include <opm/io/eclipse/ExtESmry.hpp>

#include <boost/algorithm/string.hpp>

#include <appl/qsum_cmdf.hpp>
#include <appl/derived_smry.hpp>

#include <appl/qsum_func_lib.hpp>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <boost/filesystem.hpp>

#include <omp.h>

#include <chrono>

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
namespace Qt
{
    static auto endl = ::endl;
    static auto SkipEmptyParts = QString::SkipEmptyParts;
}
#endif


static void printHelp()
{
    std::cout << "\nUsage: qsummary [file_1] [file_2] .. [file_n]  [OPTIONS] \n";

    std::cout << "\noptions: \n\n";

    std::cout << " -a   Create plot with all vectors. Useful for smaller models.  \n";
    std::cout << "      Execution of program will stop if number of charts is greater than 200 \n";
    std::cout << " -h   Print help message and exit \n";
    std::cout << " -z   Ignore summary vectors with only zero values \n";
    std::cout << " -l   Command line list to be used in command file  \n";
    std::cout << " -v   Create plot with vector. Example -v FOPR,FOPT will create \n";
    std::cout << "      one chart for each vector. Each chart holding series for all summary files. \n";
    std::cout << " -s   Separate charts on input folders. Simulation cases located in different  \n";
    std::cout << "      folders will not be placed on same chart when using this option. \n";
    std::cout << " -x   Set xrange for all charts, example  -x 2020-01,2020-03  \n";

    std::cout << "\ncommands: \n\n";

    std::cout << " :x [from_date] [to_date] - update range for time axis for active chart\n";
    std::cout << " :y [axis] [from_value] [to_value] - update range for value axis for active chart\n";
    std::cout << " :r    reload data and update all charts\n";
    std::cout << " :pdf  create pdf file (open file dialog) \n";
    std::cout << " :pdf [file_name] create pdf file save to file name. \n";
    std::cout << " :m   switch markers on or off, all series  \n";
    std::cout << " :e   exit application  \n";
    std::cout << " :ens switch to esemble mode (this part of the code is under construction)  \n";

    std::cout << "\ncontrols: \n\n";

    std::cout << " <ctrl> + c  copy visible chart to clipboard  \n";
    std::cout << " <ctrl> + r  reset chart zoom and x and y range  \n";
    std::cout << " <ctrl> + <shift> + r  reload data and update all charts  \n";
    std::cout << " <ctrl> + p  create pdf file (open file dialog)  \n";
    std::cout << " <ctrl> + o  open summary file (open file dialog)  \n";
    std::cout << " <ctrl> + x  rescale x axis, from first to last non-zero. y-axis also rescaled  \n";
    std::cout << " <ctrl> + z  rescale value axis, ignore zero  \n";
    std::cout << " <delete>   delete last series active chart  \n";
    std::cout << " <ctrl> + <delete>   delete active chart  \n";
    std::cout << " <page down>   next chart, create new if last chart already active  \n";
    std::cout << " <page down>   previous chart  \n";
    std::cout << " <home>   first chart  \n";
    std::cout << " <end>   last chart  \n";
    std::cout << " <down>  previous summary vector or previous command  \n";
    std::cout << " <up>  next summary vector or next command  \n";
    std::cout << "\n\n";

    exit(0);
}



int main(int argc, char *argv[])
{
    const int max_number_of_charts = 2000;

    int c = 0;
    bool plot_all    = false;
    bool separate    = false;
    bool ignore_zero = false;

    int max_threads  = 16;
    std::string xrange_str;
    std::string cmd_file;
    std::string cmdl_list;

    std::string smry_vect = "";

    while ((c = getopt(argc, argv, "ahf:l:v:x:n:sz")) != -1) {
        switch (c) {
        case 'h':
            printHelp();
            return 0;
        case 'v':
            smry_vect = optarg;
            break;
        case 'a':
            plot_all = true;
            break;
        case 'f':
            cmd_file = optarg;
            break;
        case 'x':
            xrange_str = optarg;
            break;
        case 'n':
            max_threads = atoi(optarg);
            break;
        case 'l':
            cmdl_list = optarg;
            break;
        case 's':
            separate = true;
            break;
        case 'z':
            ignore_zero = true;
            break;
        default:
            return EXIT_FAILURE;
        }
    }

    int argOffset = optind;

    std::replace( xrange_str.begin(), xrange_str.end(), ',', ' ');

    std::vector<std::string> arg_vect;

    for (size_t n = argOffset; n < argc; n++) {

        std::string file_arg = argv[n];

        std::filesystem::path filename(file_arg);
        std::string ext = filename.extension().string();

        auto l = file_arg.size();

        if ((ext != ".SMSPEC") && (ext != ".FSMSPEC") && (ext != ".ESMRY")) {

            if (file_arg.substr(l-1) == ".")
                file_arg = file_arg + "SMSPEC";
            else if (file_arg.size() <  7)
                file_arg = file_arg + ".SMSPEC";
            else if (file_arg.substr(l-7) != ".SMSPEC")
                file_arg = file_arg + ".SMSPEC";
        }

        l = file_arg.size();

        if (file_arg.substr(l-7) == ".SMSPEC"){
            std::string unsmry_str = file_arg.substr(0, l-7) + ".UNSMRY";
            std::filesystem::path unsmry_file(unsmry_str);

            if (std::filesystem::exists(unsmry_file))
                arg_vect.push_back(file_arg);

        } else
            arg_vect.push_back(file_arg);
    }

    std::vector<std::filesystem::path> smry_files;
    std::vector<FileType> file_type;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> lodsmry_loader;

    size_t nthreads = omp_get_max_threads();

    if (nthreads > max_threads)
        nthreads = max_threads;

    if (nthreads > arg_vect.size())
        nthreads = arg_vect.size();

    if (nthreads == 0){

        if (plot_all){
            std::cout << "\nError ! option -a can't be used without specifying one or more summary files on command line \n\n";
            exit(1);
        }

        if (smry_vect.size() > 0){
            std::cout << "\nError ! option -v can't be used without specifying one or more summary files on command line \n\n";
            exit(1);
        }

        if (cmd_file.size() > 0){
            std::cout << "\nError ! option -f can't be used without specifying one or more summary files on command line \n\n";
            exit(1);
        }

        nthreads = 1;
    }

    if ((cmdl_list.size() > 0) && (cmd_file.empty())) {
        std::cout << "\nError ! option -l must be used together with option -f \n\n";
        exit(1);
    }

    std::cout << "\nNumber of threads: " << nthreads;

    omp_set_num_threads(nthreads);

    size_t num_files = arg_vect.size();

    std::vector<std::unique_ptr<Opm::EclIO::ESmry>> esmry_vect;
    std::vector<std::unique_ptr<Opm::EclIO::ExtESmry>> lodsmry_vect;

    esmry_vect.resize(num_files);
    lodsmry_vect.resize(num_files);
    smry_files.resize(num_files);
    file_type.resize(num_files);

    auto start_open = std::chrono::system_clock::now();

    int n=0;
    #pragma omp parallel for
    for (size_t n=0; n < num_files; n++){
        std::filesystem::path filename(arg_vect[n]);
        smry_files[n] = filename;
        std::string ext = filename.extension().string();

        if (ext == ".SMSPEC") {
            file_type[n] = FileType::SMSPEC;
            
            int t = 0;
            bool open_ok = false;
            
            while (open_ok == false){
                try {
                    esmry_vect[n] = std::make_unique<Opm::EclIO::ESmry>(filename);
                    open_ok = true;
                    
                } catch (...){
                    t++;
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    
                    if ( t > 2){
                        std::string message = "Error opening SMSPEC file " + filename.string() + " in main function";
                        message = message + " after " + std::to_string(t) + " attempts";  
                        throw std::runtime_error(message);
                    }
                }                    
            }

        }  else if (ext == ".ESMRY") {
            file_type[n] = FileType::ESMRY;

            try {
                lodsmry_vect[n] = std::make_unique<Opm::EclIO::ExtESmry>(filename);
            } catch (...){
                std::string message = "Error opening ESMRY file " + filename.string() + " in main function";
                throw std::runtime_error(message);
            }
        }
    }
    
    for (size_t n=0; n < num_files; n++){
        if (file_type[n] == FileType::SMSPEC)
            esmry_loader[n] = std::move(esmry_vect[n]);
        else if (file_type[n] == FileType::ESMRY)
            lodsmry_loader[n] = std::move(lodsmry_vect[n]);
    }

    auto end_open = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = end_open-start_open;

    std::cout << " , elapsed opening " <<  elapsed_seconds.count();

    if ((cmd_file.size() > 0) && (smry_vect.size() > 0)){
        throw std::invalid_argument("not possible to combine -v and -f option");
    }


    QApplication a(argc, argv);

    const char *homedir;

    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }

    setlocale(LC_ALL,"en_US.utf8");
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    SmryAppl::input_list_type input_charts;
    SmryAppl::loader_list_type loaders;

    std::unique_ptr<DerivedSmry> derived_smry;

    if (plot_all){

        if (smry_vect.size() > 0)
            std::cout << "\n! Warning, -v option ignored since option -a (= plot all) used \n\n";

        std::vector<std::string> keyw_list;
        if (file_type[0] == FileType::SMSPEC)
            keyw_list = esmry_loader[0]->keywordList();
        else if (file_type[0] == FileType::ESMRY)
            keyw_list = lodsmry_loader[0]->keywordList();

        QSum::update_input(input_charts, keyw_list, file_type, esmry_loader, lodsmry_loader, max_number_of_charts, xrange_str);

        QSum::pre_load_smry(smry_files, input_charts, file_type, esmry_loader, lodsmry_loader, nthreads);

        if (ignore_zero)
            QSum::remove_zero_vect(smry_files, input_charts, file_type, esmry_loader, lodsmry_loader);


    } else if (smry_vect.size() > 0){

        QSum::chart_input_from_string(smry_vect, input_charts, file_type, esmry_loader, lodsmry_loader, max_number_of_charts, xrange_str);

        //QSum::print_input_charts(input_charts);

        QSum::pre_load_smry(smry_files, input_charts, file_type, esmry_loader, lodsmry_loader, nthreads);

        if (ignore_zero)
            QSum::remove_zero_vect(smry_files, input_charts, file_type, esmry_loader, lodsmry_loader);

        if (separate)
            input_charts = QSum::charts_separate_folders(smry_files, input_charts);


    } else if (cmd_file.size() > 0) {

        QsumCMDF cmdfile(cmd_file, num_files, cmdl_list);

        cmdfile.make_charts_from_cmd(input_charts, xrange_str);

        QSum::check_summary_vectors(input_charts, file_type, esmry_loader, lodsmry_loader);

        QSum::pre_load_smry(smry_files, input_charts, file_type, esmry_loader, lodsmry_loader, nthreads);

        if (separate)
            input_charts = QSum::charts_separate_folders(smry_files, input_charts);

        if (cmdfile.count_define() > 0){
            std::tuple<double,double> io_elapsed;

            derived_smry = std::make_unique<DerivedSmry>(cmdfile, file_type, esmry_loader, lodsmry_loader);

        }
    }

    std::cout << std::endl;


    loaders = std::make_tuple(smry_files, file_type, std::move(esmry_loader), std::move(lodsmry_loader));


    SmryAppl window(arg_vect, loaders, input_charts, derived_smry);

    window.resize(1400, 700);

    window.setWindowTitle("qsummary");

    window.move(300, 100);

    window.show();

    return a.exec();
}
