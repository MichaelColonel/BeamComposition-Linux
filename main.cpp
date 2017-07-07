/*
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#include <QApplication>
#include <QTranslator>
#include <QLocale>

//#include <TROOT.h>
//#include <TStyle.h>
#include <TApplication.h>

#include "mainwindow.h"

namespace {

//Int_t colors[] = { 0, 1, 2, 3, 4, 5, 6}; // #colors >= #levels - 1

} // namespace

int
main( int argc, char** argv)
{
//    gStyle->SetPalette( (sizeof(colors) / sizeof(Int_t)), colors);

    TApplication rootapp( "Qt ROOT Application", &argc, argv);
    QApplication app( argc, argv);

    QTranslator* translator = new QTranslator;
    translator->load( "BeamComposition_" + QLocale::system().name());
    app.installTranslator(translator);

    MainWindow* window = new MainWindow;
    window->show();

    int res = app.exec();

    delete window;
    delete translator;

    return res;
}
