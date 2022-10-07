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


#include <QtTest/QtTest>

#include <appl/smry_appl.hpp>
#include <tests/qsum_test_utility.hpp>

class TestQsummary: public QObject
{
    Q_OBJECT

private slots:

    void test_1a();
    void test_1b();

    void test_2a();
    void test_2b();
    void test_2c();
    void test_2d();
};


void TestQsummary::test_1a()
{
    std::vector<std::string> fname_list;
    fname_list.push_back("../tests/smry_files/SENS0.ESMRY");

    auto loaders = QSum::make_loaders(fname_list);

    std::vector<SmryAppl::vect_input_type> vect_list_c1;

    vect_list_c1 = {
                     std::make_tuple(0, "FOPR", -1, false),
                   };


    SmryAppl::input_list_type input_list;

    input_list = { std::make_tuple(vect_list_c1, ""),
                 };

    std::unique_ptr<DerivedSmry> derived_smry;

    SmryAppl window(fname_list, loaders, input_list, derived_smry);

    SmryYaxis* yaxis = window.get_smry_yaxis(0, 0);

    float a_min = static_cast<float>(yaxis->min());
    float a_max = static_cast<float>(yaxis->max());

    QVERIFY(abs(a_min - 3.0) < 1e-6);
    QVERIFY(abs(a_max - 11.0) < 1e-6);
    QVERIFY(abs(yaxis->multiplier() - 0.001) < 1e-6);
}

void TestQsummary::test_1b()
{
    std::vector<std::string> fname_list;
    fname_list.push_back("../tests/smry_files/SENS0.ESMRY");

    auto loaders = QSum::make_loaders(fname_list);

    std::vector<SmryAppl::vect_input_type> vect_list_c1;

    vect_list_c1 = {
                     std::make_tuple(0, "FOPR", -1, false),
                   };


    SmryAppl::input_list_type input_list;

    input_list = { std::make_tuple(vect_list_c1, "2019-2 2019-6"),
                 };

    std::unique_ptr<DerivedSmry> derived_smry;

    SmryAppl window(fname_list, loaders, input_list, derived_smry);

    SmryYaxis* yaxis = window.get_smry_yaxis(0, 0);

    float a_min = static_cast<float>(yaxis->min());
    float a_max = static_cast<float>(yaxis->max());

    QVERIFY(abs(a_min - 7.6) < 1e-6);
    QVERIFY(abs(a_max - 10.4) < 1e-6);
    QVERIFY(abs(yaxis->multiplier() - 0.001) < 1e-6);
}



void TestQsummary::test_2a()
{
    std::vector<std::string> fname_list;
    fname_list.push_back("../tests/smry_files/SENS0.ESMRY");

    auto loaders = QSum::make_loaders(fname_list);

    std::vector<SmryAppl::vect_input_type> vect_list_c1;

    vect_list_c1 = {
                     std::make_tuple(0, "FOPR", -1, false),
                   };


    SmryAppl::input_list_type input_list;

    input_list = { std::make_tuple(vect_list_c1, ""),
                 };

    std::unique_ptr<DerivedSmry> derived_smry;

    SmryAppl window(fname_list, loaders, input_list, derived_smry);

    SmryYaxis* yaxis = window.get_smry_yaxis(0, 0);

    float a_min = static_cast<float>(yaxis->min());
    float a_max = static_cast<float>(yaxis->max());

    QVERIFY(abs(a_min - 3.0) < 1e-6);
    QVERIFY(abs(a_max - 11.0) < 1e-6);
    QVERIFY(abs(yaxis->multiplier() - 0.001) < 1e-6);

    QLineEdit* cmdline = window.get_cmdline();

    QTest::keyClicks(cmdline, ":xrange 2019-2 2019-6");
    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_Return );

    a_min = static_cast<float>(yaxis->min());
    a_max = static_cast<float>(yaxis->max());

    QVERIFY(abs(a_min - 7.6) < 1e-6);
    QVERIFY(abs(a_max - 10.4) < 1e-6);
    QVERIFY(abs(yaxis->multiplier() - 0.001) < 1e-6);
}

void TestQsummary::test_2b()
{
    std::vector<std::string> fname_list;
    fname_list.push_back("../tests/smry_files/SENS0.ESMRY");

    auto loaders = QSum::make_loaders(fname_list);

    SmryAppl::input_list_type input_list;

    std::unique_ptr<DerivedSmry> derived_smry;

    SmryAppl window(fname_list, loaders, input_list, derived_smry);

    QLineEdit* cmdline = window.get_cmdline();

    QTest::keyClicks(cmdline, "FGOR");
    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_Return );

    SmryYaxis* yaxis = window.get_smry_yaxis(0, 0);

    float a_min = static_cast<float>(yaxis->min());
    float a_max = static_cast<float>(yaxis->max());

    QVERIFY(abs(a_min - 80) < 1e-6);
    QVERIFY(abs(a_max - 360) < 1e-6);
    QVERIFY(abs(yaxis->multiplier() - 1.0) < 1e-6);

    QTest::keyClicks(cmdline, ":xrange 2019-6 2019-9");
    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_Return );

    a_min = static_cast<float>(yaxis->min());
    a_max = static_cast<float>(yaxis->max());

    QVERIFY(abs(a_min - 160) < 1e-6);
    QVERIFY(abs(a_max - 320) < 1e-6);
    QVERIFY(abs(yaxis->multiplier() - 1.0) < 1e-6);

}


void TestQsummary::test_2c()
{
    std::vector<std::string> fname_list;
    fname_list.push_back("../tests/smry_files/SENS0.ESMRY");
    fname_list.push_back("../tests/smry_files/SENS1.ESMRY");
    fname_list.push_back("../tests/smry_files/SENS2.ESMRY");

    auto loaders = QSum::make_loaders(fname_list);

    std::vector<SmryAppl::vect_input_type> vect_list_c1;

    vect_list_c1 = {
                     std::make_tuple(0, "WBHP:PROD-1", -1, false),
                     std::make_tuple(1, "WBHP:PROD-1", -1, false),
                     std::make_tuple(2, "WBHP:PROD-1", -1, false)
                   };


    SmryAppl::input_list_type input_list;

    input_list = { std::make_tuple(vect_list_c1, ""),
                 };

    std::unique_ptr<DerivedSmry> derived_smry;

    SmryAppl window(fname_list, loaders, input_list, derived_smry);

    SmryYaxis* yaxis = window.get_smry_yaxis(0, 0);

    // :ignore zero 1 1

    float a_min = static_cast<float>(yaxis->min());
    float a_max = static_cast<float>(yaxis->max());

    QVERIFY(abs(a_min - 0.0) < 1e-6);
    QVERIFY(abs(a_max - 300.0) < 1e-6);
    QVERIFY(abs(yaxis->multiplier() - 1.00) < 1e-6);

    QLineEdit* cmdline = window.get_cmdline();

    QTest::keyClicks(cmdline, ":ignore zero 1 1");
    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_Return );

    a_min = static_cast<float>(yaxis->min());
    a_max = static_cast<float>(yaxis->max());

    QVERIFY(abs(a_min - 100.0) < 1e-6);
    QVERIFY(abs(a_max - 240.0) < 1e-6);
    QVERIFY(abs(yaxis->multiplier() - 1.00) < 1e-6);
}

void TestQsummary::test_2d()
{
    std::vector<std::string> fname_list;
    fname_list.push_back("../tests/smry_files/SENS0.ESMRY");
    fname_list.push_back("../tests/smry_files/SENS1.ESMRY");
    fname_list.push_back("../tests/smry_files/SENS2.ESMRY");

    auto loaders = QSum::make_loaders(fname_list);

    std::vector<SmryAppl::vect_input_type> vect_list_c1;

    vect_list_c1 = {
                     std::make_tuple(0, "WBHP:PROD-1", -1, false),
                     std::make_tuple(1, "WBHP:PROD-1", -1, false),
                     std::make_tuple(2, "WBHP:PROD-1", -1, false)
                   };


    SmryAppl::input_list_type input_list;

    input_list = { std::make_tuple(vect_list_c1, "2018-12 2019-8"),
                 };

    std::unique_ptr<DerivedSmry> derived_smry;

    SmryAppl window(fname_list, loaders, input_list, derived_smry);

    SmryYaxis* yaxis = window.get_smry_yaxis(0, 0);

    // :ignore zero 1 1

    float a_min = static_cast<float>(yaxis->min());
    float a_max = static_cast<float>(yaxis->max());

    QVERIFY(abs(a_min - 0.0) < 1e-6);
    QVERIFY(abs(a_max - 250.0) < 1e-6);
    QVERIFY(abs(yaxis->multiplier() - 1.00) < 1e-6);

    QLineEdit* cmdline = window.get_cmdline();

    QTest::keyClicks(cmdline, ":ignore zero 1 1");
    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_Return );

    a_min = static_cast<float>(yaxis->min());
    a_max = static_cast<float>(yaxis->max());

    QVERIFY(abs(a_min - 100.0) < 1e-6);
    QVERIFY(abs(a_max - 240.0) < 1e-6);
    QVERIFY(abs(yaxis->multiplier() - 1.00) < 1e-6);

    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_R, Qt::ControlModifier);

    a_min = static_cast<float>(yaxis->min());
    a_max = static_cast<float>(yaxis->max());

    QVERIFY(abs(a_min - 0.0) < 1e-6);
    QVERIFY(abs(a_max - 300.0) < 1e-6);
    QVERIFY(abs(yaxis->multiplier() - 1.00) < 1e-6);

    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_Z, Qt::ControlModifier);

    a_min = static_cast<float>(yaxis->min());
    a_max = static_cast<float>(yaxis->max());

    QVERIFY(abs(a_min - 100.0) < 1e-6);
    QVERIFY(abs(a_max - 240.0) < 1e-6);
    QVERIFY(abs(yaxis->multiplier() - 1.00) < 1e-6);
}


QTEST_MAIN(TestQsummary)

#include "test_scale_yaxis.moc"
