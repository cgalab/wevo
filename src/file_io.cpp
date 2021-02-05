#include <sstream>
#include <iostream>
#include <assert.h>
#include "file_io.h"
#include "sites.h"

const char *IPE_HEADER = "<?xml version=\"1.0\"?> \
<!DOCTYPE ipe SYSTEM \"ipe.dtd\"> \
<ipe version=\"70107\" creator=\"Ipe 7.1.10\"> \
    <info created=\"D:20180507200835\" modified=\"D:20180507200836\"/> \
    <ipestyle name=\"basic\"> \
        <symbol name=\"arrow/arc(spx)\"> \
            <path stroke=\"sym-stroke\" fill=\"sym-stroke\" pen=\"sym-pen\"> \
                0 0 m \
                -1 0.333 l \
                -1 -0.333 l \
                h \
            </path> \
        </symbol> \
        <symbol name=\"arrow/farc(spx)\"> \
            <path stroke=\"sym-stroke\" fill=\"white\" pen=\"sym-pen\"> \
                0 0 m \
                -1 0.333 l \
                -1 -0.333 l \
                h \
            </path> \
        </symbol> \
        <symbol name=\"arrow/ptarc(spx)\"> \
            <path stroke=\"sym-stroke\" fill=\"sym-stroke\" pen=\"sym-pen\"> \
                0 0 m \
                -1 0.333 l \
                -0.8 0 l \
                -1 -0.333 l \
                h \
            </path> \
        </symbol> \
        <symbol name=\"arrow/fptarc(spx)\"> \
            <path stroke=\"sym-stroke\" fill=\"white\" pen=\"sym-pen\"> \
                0 0 m \
                -1 0.333 l \
                -0.8 0 l \
                -1 -0.333 l \
                h \
            </path> \
        </symbol> \
        <symbol name=\"mark/circle(sx)\" transformations=\"translations\"> \
            <path fill=\"sym-stroke\"> \
                0.6 0 0 0.6 0 0 e \
                0.4 0 0 0.4 0 0 e \
            </path> \
        </symbol> \
        <symbol name=\"mark/disk(sx)\" transformations=\"translations\"> \
            <path fill=\"sym-stroke\"> \
                0.6 0 0 0.6 0 0 e \
            </path> \
        </symbol> \
        <symbol name=\"mark/fdisk(sfx)\" transformations=\"translations\"> \
            <group> \
                <path fill=\"sym-fill\"> \
                    0.5 0 0 0.5 0 0 e \
                </path> \
                <path fill=\"sym-stroke\" fillrule=\"eofill\"> \
                    0.6 0 0 0.6 0 0 e \
                    0.4 0 0 0.4 0 0 e \
                </path> \
            </group> \
        </symbol> \
        <symbol name=\"mark/box(sx)\" transformations=\"translations\"> \
            <path fill=\"sym-stroke\" fillrule=\"eofill\"> \
                -0.6 -0.6 m \
                0.6 -0.6 l \
                0.6 0.6 l \
                -0.6 0.6 l \
                h \
                -0.4 -0.4 m \
                0.4 -0.4 l \
                0.4 0.4 l \
                -0.4 0.4 l \
                h \
            </path> \
        </symbol> \
        <symbol name=\"mark/square(sx)\" transformations=\"translations\"> \
            <path fill=\"sym-stroke\"> \
                -0.6 -0.6 m \
                0.6 -0.6 l \
                0.6 0.6 l \
                -0.6 0.6 l \
                h \
            </path> \
        </symbol> \
        <symbol name=\"mark/fsquare(sfx)\" transformations=\"translations\"> \
            <group> \
                <path fill=\"sym-fill\"> \
                    -0.5 -0.5 m \
                    0.5 -0.5 l \
                    0.5 0.5 l \
                    -0.5 0.5 l \
                    h \
                </path> \
                <path fill=\"sym-stroke\" fillrule=\"eofill\"> \
                    -0.6 -0.6 m \
                    0.6 -0.6 l \
                    0.6 0.6 l \
                    -0.6 0.6 l \
                    h \
                    -0.4 -0.4 m \
                    0.4 -0.4 l \
                    0.4 0.4 l \
                    -0.4 0.4 l \
                    h \
                </path> \
            </group> \
        </symbol> \
        <symbol name=\"mark/cross(sx)\" transformations=\"translations\"> \
            <group> \
                <path fill=\"sym-stroke\"> \
                    -0.43 -0.57 m \
                    0.57 0.43 l \
                    0.43 0.57 l \
                    -0.57 -0.43 l \
                    h \
                </path> \
                <path fill=\"sym-stroke\"> \
                    -0.43 0.57 m \
                    0.57 -0.43 l \
                    0.43 -0.57 l \
                    -0.57 0.43 l \
                    h \
                </path> \
            </group> \
        </symbol> \
        <symbol name=\"arrow/fnormal(spx)\"> \
            <path stroke=\"sym-stroke\" fill=\"white\" pen=\"sym-pen\"> \
                0 0 m \
                -1 0.333 l \
                -1 -0.333 l \
                h \
            </path> \
        </symbol> \
        <symbol name=\"arrow/pointed(spx)\"> \
            <path stroke=\"sym-stroke\" fill=\"sym-stroke\" pen=\"sym-pen\"> \
                0 0 m \
                -1 0.333 l \
                -0.8 0 l \
                -1 -0.333 l \
                h \
            </path> \
        </symbol> \
        <symbol name=\"arrow/fpointed(spx)\"> \
            <path stroke=\"sym-stroke\" fill=\"white\" pen=\"sym-pen\"> \
                0 0 m \
                -1 0.333 l \
                -0.8 0 l \
                -1 -0.333 l \
                h \
            </path> \
        </symbol> \
        <symbol name=\"arrow/linear(spx)\"> \
            <path stroke=\"sym-stroke\" pen=\"sym-pen\"> \
                -1 0.333 m \
                0 0 l \
                -1 -0.333 l \
            </path> \
        </symbol> \
        <symbol name=\"arrow/fdouble(spx)\"> \
            <path stroke=\"sym-stroke\" fill=\"white\" pen=\"sym-pen\"> \
                0 0 m \
                -1 0.333 l \
                -1 -0.333 l \
                h \
                -1 0 m \
                -2 0.333 l \
                -2 -0.333 l \
                h \
            </path> \
        </symbol> \
        <symbol name=\"arrow/double(spx)\"> \
            <path stroke=\"sym-stroke\" fill=\"sym-stroke\" pen=\"sym-pen\"> \
                0 0 m \
                -1 0.333 l \
                -1 -0.333 l \
                h \
                -1 0 m \
                -2 0.333 l \
                -2 -0.333 l \
                h \
            </path> \
        </symbol> \
        <pen name=\"heavier\" value=\"0.8\"/> \
        <pen name=\"fat\" value=\"1.2\"/> \
        <pen name=\"ultrafat\" value=\"2\"/> \
        <symbolsize name=\"large\" value=\"5\"/> \
        <symbolsize name=\"small\" value=\"2\"/> \
        <symbolsize name=\"tiny\" value=\"1.1\"/> \
        <arrowsize name=\"large\" value=\"10\"/> \
        <arrowsize name=\"small\" value=\"5\"/> \
        <arrowsize name=\"tiny\" value=\"3\"/> \
        <color name=\"red\" value=\"1 0 0\"/> \
        <color name=\"green\" value=\"0 1 0\"/> \
        <color name=\"blue\" value=\"0 0 1\"/> \
        <color name=\"yellow\" value=\"1 1 0\"/> \
        <color name=\"orange\" value=\"1 0.647 0\"/> \
        <color name=\"gold\" value=\"1 0.843 0\"/> \
        <color name=\"purple\" value=\"0.627 0.125 0.941\"/> \
        <color name=\"gray\" value=\"0.745\"/> \
        <color name=\"brown\" value=\"0.647 0.165 0.165\"/> \
        <color name=\"navy\" value=\"0 0 0.502\"/> \
        <color name=\"pink\" value=\"1 0.753 0.796\"/> \
        <color name=\"seagreen\" value=\"0.18 0.545 0.341\"/> \
        <color name=\"turquoise\" value=\"0.251 0.878 0.816\"/> \
        <color name=\"violet\" value=\"0.933 0.51 0.933\"/> \
        <color name=\"darkblue\" value=\"0 0 0.545\"/> \
        <color name=\"darkcyan\" value=\"0 0.545 0.545\"/> \
        <color name=\"darkgray\" value=\"0.663\"/> \
        <color name=\"darkgreen\" value=\"0 0.392 0\"/> \
        <color name=\"darkmagenta\" value=\"0.545 0 0.545\"/> \
        <color name=\"darkorange\" value=\"1 0.549 0\"/> \
        <color name=\"darkred\" value=\"0.545 0 0\"/> \
        <color name=\"lightblue\" value=\"0.678 0.847 0.902\"/> \
        <color name=\"lightcyan\" value=\"0.878 1 1\"/> \
        <color name=\"lightgray\" value=\"0.827\"/> \
        <color name=\"lightgreen\" value=\"0.565 0.933 0.565\"/> \
        <color name=\"lightyellow\" value=\"1 1 0.878\"/> \
        <dashstyle name=\"dashed\" value=\"[4] 0\"/> \
        <dashstyle name=\"dotted\" value=\"[1 3] 0\"/> \
        <dashstyle name=\"dash dotted\" value=\"[4 2 1 2] 0\"/> \
        <dashstyle name=\"dash dot dotted\" value=\"[4 2 1 2 1 2] 0\"/> \
        <textsize name=\"large\" value=\"\\large\"/> \
        <textsize name=\"Large\" value=\"\\Large\"/> \
        <textsize name=\"LARGE\" value=\"\\LARGE\"/> \
        <textsize name=\"huge\" value=\"\\huge\"/> \
        <textsize name=\"Huge\" value=\"\\Huge\"/> \
        <textsize name=\"small\" value=\"\\small\"/> \
        <textsize name=\"footnote\" value=\"\\footnotesize\"/> \
        <textsize name=\"tiny\" value=\"\\tiny\"/> \
        <textstyle name=\"center\" begin=\"\\begin{center}\" end=\"\\end{center}\"/> \
        <textstyle name=\"itemize\" begin=\"\\begin{itemize}\" end=\"\\end{itemize}\"/> \
        <textstyle name=\"item\" begin=\"\\begin{itemize}\\item{}\" end=\"\\end{itemize}\"/> \
        <gridsize name=\"4 pts\" value=\"4\"/> \
        <gridsize name=\"8 pts (~3 mm)\" value=\"8\"/> \
        <gridsize name=\"16 pts (~6 mm)\" value=\"16\"/> \
        <gridsize name=\"32 pts (~12 mm)\" value=\"32\"/> \
        <gridsize name=\"10 pts (~3.5 mm)\" value=\"10\"/> \
        <gridsize name=\"20 pts (~7 mm)\" value=\"20\"/> \
        <gridsize name=\"14 pts (~5 mm)\" value=\"14\"/> \
        <gridsize name=\"28 pts (~10 mm)\" value=\"28\"/> \
        <gridsize name=\"56 pts (~20 mm)\" value=\"56\"/> \
        <anglesize name=\"90 deg\" value=\"90\"/> \
        <anglesize name=\"60 deg\" value=\"60\"/> \
        <anglesize name=\"45 deg\" value=\"45\"/> \
        <anglesize name=\"30 deg\" value=\"30\"/> \
        <anglesize name=\"22.5 deg\" value=\"22.5\"/> \
        <opacity name=\"10%\" value=\"0.1\"/> \
        <opacity name=\"30%\" value=\"0.3\"/> \
        <opacity name=\"50%\" value=\"0.5\"/> \
        <opacity name=\"75%\" value=\"0.75\"/> \
        <layout paper=\"320 240\" origin=\"0 0\" frame=\"320 240\"/> \
        <tiling name=\"falling\" angle=\"-60\" step=\"4\" width=\"1\"/> \
        <tiling name=\"rising\" angle=\"30\" step=\"4\" width=\"1\"/> \
    </ipestyle> \
    <page> \
";



const char *IPE_FOOTER = "</page> \
</ipe> \
        ";

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
    m_file.open(filePath, std::ofstream::out | std::ofstream::trunc);
    m_layers.insert("Labels");
}

void IpeWriter::write() {
    m_file << IPE_HEADER;
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
    m_file << IPE_FOOTER;
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
