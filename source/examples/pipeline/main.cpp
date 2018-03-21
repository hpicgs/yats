#include <QImage>
#include <QFileInfo>
#include <QDebug>

#include <yats/scheduler.h>

struct block_filter
{
    enum class Direction : int8_t { X, Y };

    block_filter(Direction direction)
        : m_direction(direction)
    {
    }

    yats::slot<QImage, "filtered_image"_id> run(yats::slot<QImage, "image"_id> image)
    {
        QImage result(image->width(), image->height(), QImage::Format::Format_RGBA8888);

        static const int size = 5;

        for (int x = 0; x < image->width(); ++x)
        {
            for (int y = 0; y < image->height(); ++y)
            {
                std::array<int, 4> color{ { 0, 0, 0, 0 } };
                for (int i = -size; i <= size; ++i)
                {
                    QColor pixelColor;
                    if (m_direction == Direction::X)
                    {
                        if (x + i < 0 || x + i >= image->width())
                        {
                            pixelColor = QColor(image->pixel(x, y));
                        }
                        else
                        {
                            pixelColor = QColor(image->pixel(x + i, y));
                        }
                    }
                    else
                    {
                        if (y + i < 0 || y + i >= image->height())
                        {
                            pixelColor = QColor(image->pixel(x, y));
                        }
                        else
                        {
                            pixelColor = QColor(image->pixel(x, y + i));
                        }
                    }

                    color[0] += pixelColor.red();
                    color[1] += pixelColor.green();
                    color[2] += pixelColor.blue();
                    color[3] += pixelColor.alpha();
                }

                color[0] /= size * 2 + 1;
                color[1] /= size * 2 + 1;
                color[2] /= size * 2 + 1;
                color[3] /= size * 2 + 1;

                result.setPixelColor(x, y, QColor(color[0], color[1], color[2], color[3]));
            }
        }

        return result;
    }

protected:
    const Direction m_direction;
};

int main(int, char **)
{
    yats::pipeline pipeline;

    // Adding tasks
    auto image_loader = pipeline.add([](yats::slot<QString, "filename"_id> filename) -> yats::output_bundle<yats::slot<QImage, "image"_id>, yats::slot<QString, "name"_id>>
    {
        QFileInfo info(filename);
        return { QImage(filename), info.baseName() };
    });

    auto x_filter = pipeline.add<block_filter>(block_filter::Direction::X);
    x_filter->add_thread_constraint(yats::thread_group("first"));
    auto y_filter = pipeline.add<block_filter>(block_filter::Direction::Y);
    y_filter->add_thread_constraint(yats::thread_group("second"));

    auto image_writer = pipeline.add([](yats::slot<QImage, "image"_id> image, yats::slot<QString, "name"_id> name)
    {
        qDebug() << "storing";
        image->save(name + "_processed.png");
        qDebug() << "finished";
    });

    // Connecting tasks
    image_loader->output<"image"_id>() >> x_filter->input<"image"_id>();

    x_filter->output<"filtered_image"_id>() >> y_filter->input<"image"_id>();
    y_filter->output<"filtered_image"_id>() >> image_writer->input<"image"_id>();

    image_loader->output<"name"_id>() >> image_writer->input<"name"_id>();

    // External in-/output
    auto writer = image_loader->mark_as_external<"filename"_id>();
    image_loader->add_listener<"name"_id>([](QString name)
    {
        qDebug() << "Processing file" << name;
    });

    yats::scheduler scheduler(std::move(pipeline));
    std::thread thread([&scheduler]() mutable
    {
        scheduler.run();
    });

    while (true)
    {
        std::string command;
        std::getline(std::cin, command);

        auto pos = command.find("--quit");
        if (pos == command.npos)
        {
            writer(QString(command.c_str()), false);
        }
        else
        {
            writer(QString(command.substr(0, pos - 1).c_str()), true);
            break;
        }
    }

    thread.join();
    return 0;
}
