
#include <glraw/MemoryProcessor.h>

#include <cassert>
#include <utility>

#include <QDebug>
#include <QImage>
#include <QDataStream>

#include <glraw/AssetInformation.h>
#include <glraw/AbstractConverter.h>
#include <glraw/filter/AbstractFilter.h>
#include <glraw/Canvas.h>

namespace glraw
{

MemoryProcessor::MemoryProcessor(std::unique_ptr<AbstractConverter> converter)
	: m_converter(std::move(converter))
{
}

bool MemoryProcessor::process(QByteArray & data, AssetInformation & info)
{
	assert(m_converter);
	assert(info.propertyExists("width"));
	assert(info.propertyExists("height"));

	if (data.isNull())
	{
		qCritical("Input image data is null.");
		return false;
	}

	canvas()->loadTexture(data, info);

	if(!applyFilter(info))
	{
		return false;
	}

	return copyImageFromGL(data, info);
}

bool MemoryProcessor::applyFilter(AssetInformation & info)
{
	for(auto& filter : m_filters)
	{
		if(!filter->process(canvas(), info))
		{
			qCritical() << "ERROR!!";
			return false;
		}
	}
	return true;
}

bool MemoryProcessor::copyImageFromGL(QByteArray & data, AssetInformation & info)
{
	data = m_converter->convert(canvas(), info);
	return !data.isEmpty();
}

void MemoryProcessor::setConverter(AbstractConverter * converter)
{
	m_converter.reset(converter);
}

void MemoryProcessor::appendFilter(std::unique_ptr<AbstractFilter> filter)
{
	m_filters.push_back(std::move(filter));
}

std::unique_ptr<Canvas> & MemoryProcessor::canvas()
{
	if(!m_canvas)
	{
		m_canvas = std::make_unique<Canvas>();
	}

	return m_canvas;
}

void MemoryProcessor::reset()
{
    m_filters.clear();
}

} // namespace glraw
