#ifndef FILE_IO_H
#define FILE_IO_H

#include <fstream>
#include <regex>
#include <set>
#ifdef ENABLE_VIEW
#include <QString>
#endif
#include "defs.h"

// x, y, w, index
using SiteData = std::tuple<int, int, int, int>;

class IpeObj {
public:
    IpeObj(const Point_2 &pnt, const std::string &layer,
            double scale = 1., const std::string &color = "black",
            const std::string &size = "normal", const std::string &label = "");
    IpeObj(const Circular_arc_point_2 &arcPnt, const std::string &layer,
            double scale = 1., const std::string &color = "black",
            const std::string &size = "normal", const std::string &label = "");
    IpeObj(const Segment_2 &seg, const std::string &layer,
            double scale = 1., const std::string &color = "black",
            const std::string &size = "normal");
    IpeObj(const Circular_arc_2 &arc, const std::string &layer,
            double scale = 1., const std::string &color = "black",
            const std::string &size = "normal");
    IpeObj(const Circle_2 &circle, const std::string &layer,
            double scale = 1., const std::string &color = "black",
            const std::string &size = "normal");

    const std::string & str() const {
        return m_str;
    }

    const std::string & layer() const {
        return m_layer;
    }

private:
    std::string m_str;
    double m_thickness;
    std::string m_layer;
};

class IpeWriter {
public:
    IpeWriter(const std::string &filePath);

    void addIpeObj(const IpeObj &ipeObj) {
        m_ipeObjs.push_back(ipeObj);
        m_layers.insert(ipeObj.layer());
    }

    void write();

private:
    void copyFile(const std::string &filePath);

    std::vector<IpeObj> m_ipeObjs;
    std::set<std::string> m_layers;
    std::ofstream m_file;
};

class FileReader {
public:
    FileReader(const std::string &filePath);

    const std::vector<SiteData> & sites() const {
        return m_sites;
    }

private:
    void readPnts();
    void readGraphml();

    std::vector<std::string> split(const std::string &str,
            const std::regex &re) const;

    std::vector<SiteData> m_sites;
    std::ifstream m_file;
};

#endif /* FILE_IO_H */

