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

#ifndef SMRY_APPL_H
#define SMRY_APPL_H
#include <QtWidgets/QGraphicsView>

#include <opm/io/eclipse/ESmry.hpp>
#include <opm/io/eclipse/ExtESmry.hpp>

#include <appl/derived_smry.hpp>

#include <filesystem>

#include <appl/smry_series.hpp>
#include <appl/smry_xaxis.hpp>
#include <appl/smry_yaxis.hpp>
#include <appl/chartview.hpp>

#include <QHBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>

#include <set>

#include <appl/derived_smry.hpp>

class DerivedSmry;

enum class FileType{ SMSPEC, ESMRY };


class SmryAppl: public QGraphicsView
{
    Q_OBJECT

public:
    // smry_ind, name, axis_ind, derived
    using vect_input_type = std::tuple<int, std::string, int, bool>;

    using char_input_type = std::tuple<std::vector<vect_input_type>, std::string>;

    using input_list_type = std::vector<char_input_type>;

    using loader_list_type =
               std::tuple< std::vector<std::filesystem::path>,
                           std::vector<FileType>,
                           std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>>,
                           std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>>
                         >;

// std::unique_ptr<DerivedSmry> derived_smry = nullptr ,
    SmryAppl(std::vector<std::string> arg_vect, loader_list_type& loaders,
             input_list_type chart_input, std::unique_ptr<DerivedSmry>& derived_smry, QWidget *parent = 0);

    void export_figure(const std::string& fname, int chart_ind);

    QLineEdit* get_cmdline() { return le_commands; };
    SmryYaxis* get_smry_yaxis(int chart_ind, int axis_ind);
    SmryXaxis* get_smry_xaxis(int chart_ind) {return axisX[chart_ind];};

    QtCharts::QChart* get_chart(int chart_ind) {return chartList[chart_ind];};
    ChartView* get_chartview(int chart_ind) {return chart_view_list[chart_ind];};

    std::vector<SmrySeries*> get_smry_series(int chart_ind);

    size_t number_of_charts() { return chartList.size(); }
    size_t number_of_series(int chart_ind) { return series[chart_ind].size(); }


protected:

    bool eventFilter(QObject *object, QEvent *event);
    void keyPressEvent(QKeyEvent * e);
    void keyReleaseEvent(QKeyEvent *event);


private slots:
    void command_modified(const QString & txt);

private:

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> m_esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> m_ext_esmry_loader;

    std::unique_ptr<DerivedSmry> m_derived_smry;

    std::vector<QColor> color_tab;
    std::vector<Qt::PenStyle> style_tab;

    int linew;
    int cmd_hist_ind;

    bool cmd_line_error;
    bool cmd_mode = false;
    bool vect_ok;
    bool axis_mode;
    bool ens_mode = false;
    bool m_smry_loaded = false;

    bool m_shift_key = false;
    bool m_ctrl_key = false;
    bool m_alt_key = false;

    std::string str_var;
    std::string cmd_var;

    size_t p1;
    size_t p2;
    size_t vect_ind;
    size_t smry_ind;

    int chart_ind;

    QHBoxLayout* hbox1;
    QGridLayout* grid1;
    QStackedWidget *stackedWidget;

    QLineEdit *le_commands;
    QLabel *lbl_plot;
    QLabel *lbl_num;
    QLabel* lbl_cmd;
    QLabel* lbl_rootn;


    std::vector<QtCharts::QChart*> chartList;
    std::vector<ChartView*> chart_view_list;

    std::vector<SmryXaxis*> axisX;
    std::vector<std::vector<SmryYaxis*>> axisY;
    std::vector<std::vector<SmrySeries*>> series;
    std::vector<std::vector<QLineSeries*>> ens_series;

    std::map<SmrySeries*, SmryYaxis*> yaxis_map;

    std::vector<std::string> command_hist;
    std::vector<std::string> root_name_list;

    std::vector<std::filesystem::file_time_type> file_stamp_vector;
    std::vector<std::filesystem::path> m_smry_files;
    std::vector<FileType> m_file_type;

    std::vector<int> num_yaxis;
    std::vector<std::vector<std::string>> yaxis_units;

    using vectorEntry = std::tuple<std::string, std::string, std::string>;

    // id, name, vect_entry, unit, yaxis, is_derived
    using SeriesEntry = std::tuple<int, std::string, vectorEntry, std::string, int, bool>;
    using ChartEntry = std::vector<SeriesEntry>;

    std::vector<ChartEntry> charts_list;

    std::vector<std::string> vect_lookup_list;

    std::vector<std::vector<std::string>> vect_list;

    void initColorAndStyle();
    void create_charts_from_input ( const input_list_type& chart_input );
    void init_new_chart();
    bool add_new_series ( int chart_ind, int smry_ind, std::string vect_name, int vaxis_ind = -1, bool is_derived = false);
    bool add_new_ens_series ( int chart_ind, std::string vect_name, int vaxis_ind = -1);

    void update_chart_labels();

    void delete_chart(int chart_ind);

    void update_axis_range(SmryYaxis* axis);
    void update_xaxis_range(SmryXaxis* axis);

    void adjust_yaxis_props(SmryYaxis* axis, double& min, double& max);

    bool update_yrange(const std::string& cmd_str);

    void update_all_yaxis(const std::tuple<double, double>& min_max_range, int chart_ind,
                          bool ignore_zero = false);

    void update_chart_title_and_legend(int chart_ind);
    bool reload_and_update_charts();
    void reset_axis_state(int chart_index, const std::vector<std::vector<QDateTime>>& xrange_state);

    void delete_last_series();

    std::vector<std::string> make_well_list(int chart_ind, const std::string& filt);
    std::vector<std::string> make_group_list(int chart_ind, const std::string& filt );
    std::vector<std::string> make_aquifer_list(int chart_ind, const std::string& filt );

    void make_aquifer_charts ( std::string filt );
    void make_well_charts ( std::string filt );
    void make_group_charts ( std::string filt );

    std::vector<std::string> split_string ( std::string str_arg );

    std::vector<std::string> get_iter_sets(std::set<std::string>& items_set,
                                                 std::set<int>& smry_ind_set,
                                                 std::vector<int>& id_list, char c);

    std::vector<std::string> get_item_list(std::string filt, const std::vector<std::string>& vect_name_list,
                                    const std::vector<int>& id_list);

    vectorEntry make_vector_entry ( std::string vect_name );
    float calc_p90(const std::vector<float>& data);

    void modifiy_vect_lookup();
    bool is_number(const std::string &s);
    void acceptAutoComlete();

    void print_pdf(QString& fileName);

    void add_cmd_to_hist(std::string var);
    void reset_cmdline();

    void handle_delete_series();

    template <typename T>
    bool reopen_loader(int n, std::unique_ptr<T>& smry);

    bool has_smry_vect(int smry_ind, const std::string& keystr);
    const std::vector<float>& get_smry_vect(int case_ind, std::string& keystr);

    int max_vect_chart(input_list_type chart_input);

    template <typename T>
    bool double_check_well_vector(std::string& vect_name, std::unique_ptr<T>& smry);

    void copy_to_clipboard();
    void switch_markes();

    void update_full_xrange(int chart_index);

    void select_first_chart();
    void select_last_chart();

    std::tuple<QDateTime, QDateTime> calc_min_max_dt(int chart_ind);
};

#endif
