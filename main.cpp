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

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <boost/filesystem.hpp>

#include <omp.h>


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

    std::cout << " :xrange [from_date] [to_date] - update range for time axis for active chart\n";
    std::cout << " :yrange [axis] [from_value] [to_value] - update range for value axis for active chart\n";
    std::cout << " :r    reload data and update all charts\n";
    std::cout << " :pdf  create pdf file (open file dialog) \n";
    std::cout << " :pdf [file_name] create pdf file save til file name. \n";
    std::cout << " :m   switch markers on or off, all series  \n";
    std::cout << " :e   exit application  \n";
    std::cout << " :ens switch to esemble mode (this part of the code is under construction)  \n";

    std::cout << "\ncontrols: \n\n";

    std::cout << " <ctrl> + r  reset chart zoom and x and y range  \n";
    std::cout << " <shift> + r  reload data and update all charts  \n";
    std::cout << " <ctrl> + p  create pdf file (open file dialog)  \n";
    std::cout << " <ctrl> + o  open summary file (open file dialog)  \n";
    std::cout << " <delete>   delete last series active chart  \n";
    std::cout << " <ctrl> + <delete>   delete active chart  \n";
    std::cout << " <page down>   next chart, create new if last chart already active  \n";
    std::cout << " <page down>   previous chart  \n";
    std::cout << " <down>  previous summary vector or previous command  \n";
    std::cout << " <up>  next summary vector or next command  \n";
    std::cout << "\n\n";

    exit(0);
}

// Smart pointer to log file
QScopedPointer<QFile>   m_logFile;


void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // Open stream file writes
    QTextStream out(m_logFile.data());

    // Write the date of recording
    out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ");

    // By type determine to what level belongs message
    switch (type)
    {
    case QtInfoMsg:     out << "Inf "; break;
    case QtDebugMsg:    out << "Dbg "; break;
    case QtWarningMsg:  out << "Warn "; break;
    case QtCriticalMsg: out << "Crit "; break;
    case QtFatalMsg:    out << "Fatal Error"; break;
    }

    out <<  ": "  << msg << Qt::endl;
    out.flush();    // Clear the buffered data
}

template <typename T>
void list_vectors(const std::unique_ptr<T>& loader)
{
    std::vector<std::string> list;

    list = loader->keywordList();

    for (size_t n = 0; n < list.size(); n++) {
        std::cout << std::setw(20) << list[n];

        if (((n+1) % 5)==0) {
            std::cout << std::endl;
        }
    }

    std::cout << std::endl;

    std::cout << "\nnumber of vectors is " << list.size() << "\n\n";
}

template <typename T>
bool has_nonzero(std::unique_ptr<T>& smry, const std::string key)
{
    if ((key == "TIMESTEP") && (smry->all_steps_available()))
        return true;

    if (!smry->hasKey(key))
        return false;

    auto vect = smry->get(key);

    for (auto v : vect)
        if (abs(v) > 0.0)
            return true;

    return false;
}


void update_input(SmryAppl::input_list_type& input_charts,
                  const std::vector<std::string>& keyw_list,
                  const std::vector<FileType>& file_type,
                  std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>>& esmry_loader,
                  std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>>& lodsmry_loader,
                  const int max_number_of_charts,
                  const std::string& xrange
                 )
{
    for (auto v : keyw_list) {

        std::vector<SmryAppl::vect_input_type> vect_list;

        for (size_t n = 0; n < file_type.size(); n ++){

            bool hasVect;

            if (file_type[n] == FileType::SMSPEC)
                hasVect = esmry_loader[n]->hasKey(v);
            else if (file_type[n] == FileType::ESMRY)
                hasVect = lodsmry_loader[n]->hasKey(v);

            if (hasVect)
                vect_list.push_back(std::make_tuple (n, v, -1, false));
        }

        if (vect_list.size() > 0){
            SmryAppl::char_input_type chart;
            chart = std::make_tuple(vect_list, xrange);
            input_charts.push_back(chart);
        }

        if (input_charts.size() > max_number_of_charts)
            throw std::invalid_argument("exceeds maximum number of charts ");
    }
}


std::vector<std::string> split(const std::string& line, const std::string& delim)
{
    std::vector<std::string> res;

    int p0 = 0;
    bool end_of_line = false;

    while (!end_of_line) {

        int p1 = line.find_first_of(delim, p0);

        if (p1 != std::string::npos ) {
            std::string tmp = line.substr(p0, p1-p0);
            if (tmp.size()>0)
                res.push_back(tmp);
        } else {
            end_of_line = true;
            std::string tmp = line.substr(p0);
            if (tmp.size()>0)
                res.push_back(line.substr(p0));
        }

        p0 = p1 + 1;
    }

    return res;
}


SmryAppl::input_list_type  charts_separate_folders(const std::vector<std::filesystem::path>& smry_files,
                                                   SmryAppl::input_list_type input_charts)
{
    SmryAppl::input_list_type updated_chart_input;
    std::vector<std::string> folderList;

    for (auto f: smry_files)
        folderList.push_back(f.parent_path().string());

    int cind = 0;

    std::vector<std::vector<int>> smry_ind_vect;
    std::vector<std::vector<std::string>> key_vect;
    std::vector<std::string> xrange_str_vect;

    for (auto element: input_charts) {

        auto series_vector = std::get<0>(element);
        auto xrange_str = std::get<1>(element);

        std::vector<std::string> flist;
        int cur_smry_ind;

        for (size_t n = 0; n < series_vector.size(); n++) {
            int smry_ind = std::get<0>(series_vector[n]);

            if (n == 0) {
                smry_ind_vect.push_back({});
                key_vect.push_back({});
                xrange_str_vect.push_back(xrange_str);

            } else {
                if ((smry_ind != cur_smry_ind) && (folderList[smry_ind] != folderList[cur_smry_ind])) {
                    cind++;
                    smry_ind_vect.push_back({});
                    key_vect.push_back({});
                    xrange_str_vect.push_back(xrange_str);
                }
            }

            smry_ind_vect.back().push_back(std::get<0>(series_vector[n]));
            key_vect.back().push_back(std::get<1>(series_vector[n]));

            cur_smry_ind = smry_ind;
        }

        cind++;
    }

    for (size_t c = 0; c < smry_ind_vect.size(); c++){

        SmryAppl::char_input_type chart_input;
        std::vector<SmryAppl::vect_input_type> vect_input_list;

        for (size_t n = 0; n < smry_ind_vect[c].size(); n++){
            std::tuple<int, std::string, int, bool> vect;
            vect = std::make_tuple(smry_ind_vect[c][n], key_vect[c][n], -1, false);
            vect_input_list.push_back(vect);
        }

        chart_input = std::make_tuple(vect_input_list, xrange_str_vect[c]);

        updated_chart_input.push_back(chart_input);
    }

    return updated_chart_input;
}


void print_input_charts(const SmryAppl::input_list_type& input_charts)
{

    std::cout << "number of charts: " << input_charts.size() << std::endl;

    for ( size_t c = 0; c < input_charts.size(); c++ ) {

        std::cout << "chart : " << c << std::endl;

        std::vector<SmryAppl::vect_input_type> vect_input;
        vect_input = std::get<0>(input_charts[c]);

        for ( size_t i=0; i < vect_input.size(); i++ ) {
            int n = std::get<0> ( vect_input[i] );
            std::string vect_name = std::get<1> ( vect_input[i] );
            bool is_derived = std::get<3> ( vect_input[i] );

            std::cout << "smry_ind= " << n << " > vect_name: " << vect_name << "  ";
            std::cout << std::boolalpha << is_derived << std::endl;
        }
    }
}

template <typename T>
bool double_check_well_vector(std::string& vect_name, std::unique_ptr<T>& smry)
{
    auto p = vect_name.find(":");
    auto wname = vect_name.substr(p + 1);

    if (wname.size() > 8)
        wname = wname.substr(0,8);

    std::string pattern = vect_name.substr(0, p + 1) + wname + "*";

    auto vlist = smry->keywordList(pattern);

    if (vlist.size() == 1){
        vect_name = vlist[0];
        return true;
    } else {
        return false;
    }

    return true;
}



void check_summary_vectors(SmryAppl::input_list_type& input_charts,
                   const std::vector<FileType>& file_type,
                   std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>>& esmry_loader,
                   std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>>& lodsmry_loader)
{

    for ( size_t c = 0; c < input_charts.size(); c++ ) {

        std::vector<SmryAppl::vect_input_type> vect_input;
        vect_input = std::get<0>(input_charts[c]);

        for ( size_t i = 0; i < vect_input.size(); i++ ) {
            int n = std::get<0> ( vect_input[i] );
            std::string vect_name = std::get<1> ( vect_input[i] );
            int axis = std::get<2> ( vect_input[i] );
            bool is_derived = std::get<3> ( vect_input[i] );

            if (!is_derived) {

                if (file_type[n] == FileType::SMSPEC) {

                    if (!esmry_loader[n]->hasKey(vect_name)) {
                        if (vect_name.substr(0,1) == "W") {
                            if (!double_check_well_vector(vect_name, esmry_loader[n]))
                                throw std::invalid_argument("not able to load smry vector " + vect_name);
                        } else {
                            throw std::invalid_argument("not able to load smry vector " + vect_name);
                        }

                        vect_input[i] = std::make_tuple(n, vect_name, axis, false);
                        auto xrange_str = std::get<1>(input_charts[c]);
                        input_charts[c] = std::make_tuple(vect_input, xrange_str);
                    }

                } else if (file_type[n] == FileType::ESMRY) {

                    if (!lodsmry_loader[n]->hasKey(vect_name)) {
                        if (vect_name.substr(0,1) == "W") {
                            if (!double_check_well_vector(vect_name, lodsmry_loader[n]))
                                throw std::invalid_argument("not able to load smry vector " + vect_name);
                        } else {
                            throw std::invalid_argument("not able to load smry vector " + vect_name);
                        }

                        vect_input[i] = std::make_tuple(n, vect_name, axis, false);
                        auto xrange_str = std::get<1>(input_charts[c]);
                        input_charts[c] = std::make_tuple(vect_input, xrange_str);

                    }
                } else {
                    throw std::invalid_argument("unknown file type");
                }
            }
        }
    }
}


void pre_load_smry(const std::vector<std::filesystem::path>& smry_files,
                   const SmryAppl::input_list_type& input_charts,
                   const std::vector<FileType>& file_type,
                   std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>>& esmry_loader,
                   std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>>& lodsmry_loader,
                   const size_t nthreads)
{
    std::vector<std::vector<std::string>> smry_pre_load;

    for (size_t n = 0; n < smry_files.size(); n++)
        smry_pre_load.push_back({"TIME"});

    for (size_t c = 0; c < input_charts.size(); c++) {
        auto vect_input = std::get<0>(input_charts[c]);

        for (size_t s = 0; s < vect_input.size(); s++) {
            auto smry_ind = std::get<0>(vect_input[s]);
            auto vect_name = std::get<1>(vect_input[s]);
            auto is_derived = std::get<3>(vect_input[s]);

            if (!is_derived)
                smry_pre_load[smry_ind].push_back(vect_name);
        }
    }

    omp_set_num_threads(nthreads);

    auto start_load = std::chrono::system_clock::now();

    #pragma omp parallel for
    for (size_t n = 0; n < smry_files.size(); n++){
        if (smry_pre_load[n].size() > 0)
            if (file_type[n] == FileType::SMSPEC){
                esmry_loader[n]->loadData(smry_pre_load[n]);
            } else if (file_type[n] == FileType::ESMRY){
                lodsmry_loader[n]->loadData(smry_pre_load[n]);
            }
    }

    auto end_load = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = end_load-start_load;

    std::cout << ", loading: " <<  elapsed_seconds.count();
}

void remove_zero_vect(const std::vector<std::filesystem::path>& smry_files,
                   SmryAppl::input_list_type& input_charts,
                   const std::vector<FileType>& file_type,
                   std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>>& esmry_loader,
                   std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>>& lodsmry_loader)
{

    int c = static_cast<int>(input_charts.size()) -1;

    while (c > -1){

        auto vect_input = std::get<0>(input_charts[c]);
        auto xrange_str = std::get<1>(input_charts[c]);

        int n = static_cast<int>(vect_input.size()) -1 ;

        while (n > -1){
            auto smry_ind = std::get<0>(vect_input[n]);
            auto vect_name = std::get<1>(vect_input[n]);

            bool nonzero;

            if (file_type[n] == FileType::SMSPEC)
                nonzero = has_nonzero(esmry_loader[n], vect_name);
            else if (file_type[n] == FileType::ESMRY)
                nonzero = has_nonzero(lodsmry_loader[n], vect_name);

            if (!nonzero)
                vect_input.erase( vect_input.begin() + n );

            n--;
        }

        if (vect_input.size() > 0)
            input_charts[c] = std::make_tuple(vect_input, xrange_str);
        else
            input_charts.erase( input_charts.begin() + c );

        c --;
    }
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
            esmry_vect[n] = std::make_unique<Opm::EclIO::ESmry>(filename);

        }  else if (ext == ".ESMRY") {
            file_type[n] = FileType::ESMRY;
            lodsmry_vect[n] = std::make_unique<Opm::EclIO::ExtESmry>(filename);
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

    std::string log_file(homedir);
    log_file = log_file + "/qsummary.log";

    m_logFile.reset(new QFile(QString::fromStdString(log_file)));

    // Open the file logging
    m_logFile.data()->open(QFile::Append | QFile::Text);

    qInstallMessageHandler(messageHandler);

    qInfo() << " ** Starting application ** ";

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

        update_input(input_charts, keyw_list, file_type, esmry_loader, lodsmry_loader, max_number_of_charts, xrange_str);

        //print_input_charts(input_charts);
        pre_load_smry(smry_files, input_charts, file_type, esmry_loader, lodsmry_loader, nthreads);

        if (ignore_zero)
            remove_zero_vect(smry_files, input_charts, file_type, esmry_loader, lodsmry_loader);


    } else if (smry_vect.size() > 0){

        boost::to_upper(smry_vect);
        std::vector<std::string> vect_list;

        int p1 = 0;
        int p2 = smry_vect.find_first_of(",");

        while (p2 > -1) {
            vect_list.push_back(smry_vect.substr(p1, p2 - p1));
            p1 = p2 + 1;
            p2 = smry_vect.find_first_of(",", p1);
        }

        vect_list.push_back(smry_vect.substr(p1));

        for (auto vect : vect_list){

            int p = vect.find_first_of("*");

            std::vector<std::string> keyw_list;

            if (p > -1){

                if (file_type[0] == FileType::SMSPEC)
                    keyw_list = esmry_loader[0]->keywordList(vect);
                else if (file_type[0] == FileType::ESMRY)
                    keyw_list = lodsmry_loader[0]->keywordList(vect);

            } else {
                keyw_list.push_back(vect);
            }

            update_input(input_charts, keyw_list, file_type, esmry_loader, lodsmry_loader, max_number_of_charts, xrange_str);

            //if (ignore_zero)
            //    remove_zero_vect(smry_files, input_charts, file_type, esmry_loader, lodsmry_loader);
        }

        pre_load_smry(smry_files, input_charts, file_type, esmry_loader, lodsmry_loader, nthreads);

        if (ignore_zero)
            remove_zero_vect(smry_files, input_charts, file_type, esmry_loader, lodsmry_loader);

        if (separate)
            input_charts = charts_separate_folders(smry_files, input_charts);


    } else if (cmd_file.size() > 0) {

        QsumCMDF cmdfile(cmd_file, num_files, cmdl_list);

        cmdfile.make_charts_from_cmd(input_charts, xrange_str);

        //print_input_charts(input_charts);

        check_summary_vectors(input_charts, file_type, esmry_loader, lodsmry_loader);

        pre_load_smry(smry_files, input_charts, file_type, esmry_loader, lodsmry_loader, nthreads);

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
