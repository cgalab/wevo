#include <iostream>
#include <boost/program_options.hpp>
#ifdef ENABLE_VIEW
#include <QApplication>
#endif
#include "view.h"
#include "mwvd.h"
#include "log.h"

namespace po = boost::program_options;

int main(int argc, char *argv[]) {
#ifdef ENABLE_LOGGING
    Log::init();
#endif
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help", "produce help message")
            ("input-file", po::value<std::string>(), "input file")
            ("ipe-file", po::value<std::string>(), "write output to Ipe file")
            ("csv-file", po::value<std::string>(), "append runtime data to CSV file")
            ("view", "open OpenGL interface")
            ;

    po::positional_options_description p;
    p.add("input-file", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    if (vm.count("input-file")) {
        bool bEnableView = vm.count("view");

        const auto inFile = vm["input-file"].as<std::string>();
        std::string ipeFile, csvFile;
        if (vm.count("ipe-file")) {
            ipeFile = vm["ipe-file"].as<std::string>();
        }

        if (vm.count("csv-file")) {
            csvFile = vm["csv-file"].as<std::string>();
        }

        const auto vorDiag = VorDiag{inFile, true, ipeFile, csvFile,
                                     bEnableView};

#ifdef ENABLE_VIEW
        if (bEnableView) {
            QApplication app(argc, argv);
            QCoreApplication::setApplicationName("WeVo");
            QCoreApplication::setApplicationVersion(QT_VERSION_STR);
            MainWindow mainWindow;
            
            for (const auto &site : vorDiag.sites()) {
                mainWindow.addItem(new SiteGraphicsItem{site});
            }

            for (const auto &val : vorDiag.offCircs()) {
                const auto offCirc = val.second;
                mainWindow.addItem(new OffCircGraphicsItem{offCirc});
            }

            mainWindow.addItem(new VorDiagGraphicsItem{vorDiag});
            mainWindow.show();

            return app.exec();
        }
#endif
    }

    return 0;
}
