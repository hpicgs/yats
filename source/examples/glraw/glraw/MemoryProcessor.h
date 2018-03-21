#pragma once

#include <memory>
#include <vector>

#include <QByteArray>

#include <glraw/AbstractConverter.h>
#include <glraw/filter/AbstractFilter.h>

#include <glraw/glraw_api.h>


namespace glraw
{

class AssetInformation;
class Canvas;

class GLRAW_API MemoryProcessor
{
public:
	MemoryProcessor(std::unique_ptr<AbstractConverter> converter);
	virtual ~MemoryProcessor() = default;

	bool process(QByteArray & data, AssetInformation & info);

	void appendFilter(std::unique_ptr<AbstractFilter> filter);
	void setConverter(AbstractConverter * converter);

	void reset();

protected:

	bool applyFilter(AssetInformation & info);
	bool copyImageFromGL(QByteArray & data, AssetInformation & info);

	std::vector<std::unique_ptr<AbstractFilter>> m_filters;
	std::unique_ptr<AbstractConverter> m_converter;

	std::unique_ptr<Canvas> & canvas();

private:

	std::unique_ptr<Canvas> m_canvas;
};

} // namespace glraw
