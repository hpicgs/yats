
#include <glraw/ConvertManager.h>

#include <cassert>
#include <utility>

#include <QDebug>
#include <QFile>
#include <QImage>
#include <QDataStream>

#include <glraw/AssetInformation.h>
#include <glraw/AbstractConverter.h>
#include <glraw/filter/AbstractFilter.h>


namespace glraw
{

ConvertManager::ConvertManager(std::unique_ptr<FileWriter> writer, std::unique_ptr<AbstractConverter> converter)
	: MemoryProcessor(std::move(converter))
	, m_writer(std::move(writer))
{
}
   
bool ConvertManager::process(const QString & sourcePath)
{
	if (!QFile::exists(sourcePath))
	{
		qDebug("Input file does not exist.");
		return false;
	}

	QImage image(sourcePath);
	if(image.isNull())
	{
		qDebug("Loading image from input file failed.");
		return false;
	}

	AssetInformation info = generateAssetInformation(image);
	canvas()->loadTextureFromImage(image);

	if(!applyFilter(info))
	{
		qDebug("Error while applying the image filter.");
		return false;
	}

	QByteArray imageData;
	if(!copyImageFromGL(imageData, info))
	{
		qDebug("Error while extracting the image data from GPU.");
		return false;
	}

	return m_writer->write(imageData, info, sourcePath);
}

void ConvertManager::setWriter(FileWriter * writer)
{
    m_writer.reset(writer);
}

AssetInformation ConvertManager::generateAssetInformation(const QImage & image)
{
	AssetInformation info;
	info.setProperty("width", image.width());
	info.setProperty("height", image.height());

	return info;
}

} // namespace glraw
