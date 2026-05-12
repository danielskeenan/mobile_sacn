/**
 * @file gen_webui_qrc.cpp
 *
 * Generate a .qrc file for the webui.
 *
 * Qt's RCC -project has signigicant shortcomings: it can't define a resource prefix or alias per file.
 *
 * This program will write the XML with the correct access prefix and aliasing.
 *
 * @author Dan Keenan
 * @date 3/14/26
 * @copyright Apache-2.0
 */

#include <filesystem>
#include <iostream>
#include <QFile>
#include <QXmlStreamWriter>

static const auto kPrefix = QStringLiteral("/webui");

int main(int argc, char *argv[])
{
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <webui path> <output qrc path>" << std::endl;
        return 1;
    }
    std::filesystem::path webuiPath(argv[1]);
    std::filesystem::path qrcPath(argv[2]);

    QFile output(QString::fromStdString(qrcPath.string()));
    if (!output.open(QFile::WriteOnly | QFile::Text)) {
        std::cerr << "Error opening output file: " << qPrintable(output.errorString()) << std::endl;
        return 1;
    }
    QXmlStreamWriter writer(&output);
    writer.setAutoFormatting(true);

    // Begin.
    writer.writeStartDocument();
    writer.writeStartElement("RCC");
    writer.writeStartElement("qresource");
    writer.writeAttribute("prefix", kPrefix);

    // Add files.
    for (const auto &file : std::filesystem::recursive_directory_iterator(webuiPath)) {
        if (!file.is_regular_file()) {
            continue;
        }
        const auto alias = std::filesystem::relative(file.path(), webuiPath);
        const auto path = std::filesystem::relative(file.path(), qrcPath.parent_path());
        writer.writeStartElement("file");
        writer.writeAttribute("alias", QString::fromStdString(alias.string()));
        writer.writeCharacters(QString::fromStdString(path.string()));
        writer.writeEndElement(); // file
    }

    // End.
    writer.writeEndElement(); // qresource
    writer.writeEndElement(); // RCC
    writer.writeEndDocument();

    return 0;
}
