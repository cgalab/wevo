#include <sstream>
#include <iostream>
#include <assert.h>
#include "file_io.h"
#include "sites.h"

const char *IPE_HEADER_PATH = "ipe/header.ipe";
const char *IPE_FOOTER_PATH = "ipe/footer.ipe";

IpeObj::IpeObj(const Point_2 &pnt, const std::string &layer,
               double scale, const std::string &color,
               const std::string &size, const std::string &label)
: m_layer{layer}
{
    const auto arcPnt = Circular_arc_point_2{pnt};
    m_str = IpeObj(arcPnt, layer, scale, color, size, label).str();
}

IpeObj::IpeObj(const Circular_arc_point_2 &arcPnt, const std::string &layer,
       double scale, const std::string &color,
       const std::string &size, const std::string &label)
: m_layer{layer}
{
    std::stringstream strs;
    strs << "<use layer=\"" << m_layer << "\" name=\"mark/disk(sx)\" pos=\""
            << CGAL::to_double(arcPnt.x()) * scale << " " 
            << CGAL::to_double(arcPnt.y()) * scale
            << "\" size=\"" << size << "\" stroke=\"" << color << "\"/>\n";

    if (!label.empty()) {
        strs << "<text layer=\"Labels\" transformations=\"translations\" pos=\""
                << CGAL::to_double(arcPnt.x()) * scale << " " 
                << CGAL::to_double(arcPnt.y()) * scale
                << "\" stroke=\"" << color 
                << "\" type=\"Label\" valign=\"baseline\">"
                << label << "</text>\n";
    }
    
    m_str = strs.str();
}

IpeObj::IpeObj(const Segment_2 &seg, const std::string &layer, double scale, 
               const std::string &color, const std::string &size)
: m_layer{layer}
{
    std::stringstream strs;
    strs << "<path layer=\"" << m_layer << "\" stroke=\"" << color 
            << "\" pen=\"" << size << "\">\n"
            << CGAL::to_double(seg.source().x()) * scale << " " 
            << CGAL::to_double(seg.source().y()) * scale << " m\n"
            << CGAL::to_double(seg.target().x()) * scale << " " 
            << CGAL::to_double(seg.target().y()) * scale << " l\n"
            << "</path>\n";
    
    m_str = strs.str();
}

IpeObj::IpeObj(const Circular_arc_2 &arc, const std::string &layer, double scale, 
               const std::string &color, const std::string &size)
: m_layer{layer}
{
    std::stringstream strs;
    strs << "<path layer=\"" << m_layer << "\" stroke=\"" << color 
            << "\" pen=\"" << size << "\">\n"
            << CGAL::to_double(arc.source().x()) * scale << " " 
            << CGAL::to_double(arc.source().y()) * scale << " m\n"
            << std::sqrt(CGAL::to_double(arc.squared_radius())) * scale << " 0 0 " 
            << std::sqrt(CGAL::to_double(arc.squared_radius())) * scale << " "
            << CGAL::to_double(arc.center().x()) * scale << " " 
            << CGAL::to_double(arc.center().y()) * scale << " "
            << CGAL::to_double(arc.target().x()) * scale << " " 
            << CGAL::to_double(arc.target().y()) * scale << " a\n" 
            << "</path>\n";
    
    m_str = strs.str();
}

IpeObj::IpeObj(const Circle_2 &circle, const std::string &layer, double scale,
               const std::string &color, const std::string &size)
: m_layer{layer}
{
    std::stringstream strs;
    strs << "<path layer=\"" << m_layer << "\" stroke=\"" << color 
            << "\" pen=\"" << size << "\">\n"
            << std::sqrt(CGAL::to_double(circle.squared_radius())) * scale << " 0 0 " 
            << std::sqrt(CGAL::to_double(circle.squared_radius())) * scale << " "
            << CGAL::to_double(circle.center().x()) * scale << " " 
            << CGAL::to_double(circle.center().y()) * scale << " e\n"
            << "</path>\n";
    
    m_str = strs.str();
}

IpeWriter::IpeWriter(const std::string &filePath) {
    m_file.open(filePath, std::ofstream::out | std::ofstream::app);
    m_layers.insert("Labels");
}

void IpeWriter::write() {
    copyFile(IPE_HEADER_PATH);
    for (const auto &layer : m_layers)
        m_file << "<layer name=\"" << layer << "\"/>\n";

    const auto fToStr = [this]() {
        std::string str;
        for (const auto &layer : m_layers) {
            str.append(layer);
            str.append(" ");
        }

        return str;
    };
    m_file << "<view layers=\"" << fToStr()
            << "\" active=\"" << fToStr() << "\"/>";
    for (const auto &ipeObj : m_ipeObjs)
        m_file << ipeObj.str();
    copyFile(IPE_FOOTER_PATH);
}

void IpeWriter::copyFile(const std::string &filePath) {
    std::ifstream src;
    src.open(filePath, std::ofstream::in);
    m_file << src.rdbuf();
}

FileReader::FileReader(const std::string &filePath) {
    std::cout << "Opening file " << filePath << "\n";
    m_file.open(filePath, std::ifstream::in);
    const auto &suffix = filePath.substr(filePath.find_last_of(".") + 1);

    if (suffix == "pnts") {
        readPnts();
    }
}

void FileReader::readPnts() {
    if (m_file.is_open()) {
        long siteId = 0;
        std::string line;
        while (std::getline(m_file, line)) {
            const auto strs = split(line, std::regex("\\s+"));
            CGAL_assertion(strs.size() == 3);
#ifdef ENABLE_VIEW
            int x = QString::fromStdString(strs.at(0)).toInt(),
                    y = QString::fromStdString(strs.at(1)).toInt(),
                    w = QString::fromStdString(strs.at(2)).toInt();
#else
            int x = std::stoi(strs.at(0)),
                    y = std::stoi(strs.at(1)),
                    w = std::stoi(strs.at(2));
#endif

            m_sites.push_back(std::make_shared<PntSite>(Point_2{x, y}, w, siteId++));
        }
    }

    std::sort(m_sites.begin(), m_sites.end(),
              [](const SitePtr &lhs, const SitePtr & rhs) {
                  return lhs->weight() > rhs->weight();
              });
}

std::vector<std::string> FileReader::split(const std::string &str,
                                           const std::regex &re) const {
    return std::vector<std::string>{std::sregex_token_iterator(str.begin(), 
                                                               str.end(), 
                                                               re, -1),
                                    std::sregex_token_iterator()};
}
