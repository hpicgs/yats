#pragma once

#include <QString>

#include <glraw/glraw_api.h>

#include <glraw/MemoryProcessor.h>
#include <glraw/AssetInformation.h>
#include <glraw/FileWriter.h>

namespace glraw
{

class GLRAW_API ConvertManager : public MemoryProcessor
{
public:
    ConvertManager(std::unique_ptr<FileWriter> writer, std::unique_ptr<AbstractConverter> converter);

	bool process(const QString & sourcePath);
    
	void setWriter(FileWriter * writer);

protected:
    
	static AssetInformation generateAssetInformation(const QImage & image);

    std::unique_ptr<FileWriter> m_writer;
};

} // namespace glraw
