#include "window.h"

#include <iostream>

#include <globjects/globjects.h>
#include <globjects/Texture.h>
#include <globjects/VertexAttributeBinding.h>
#include <globjects/base/StaticStringSource.h>

#include <glbinding/gl/gl.h>
#include <glbinding/ContextInfo.h>
#include <glbinding/Version.h>

#include <glm/vec2.hpp>

#include <QImage>
#include <QFileInfo>

#include <yats/scheduler.h>

using namespace gl;

namespace
{
    static const char* fragmentShaderSource =
        R"(#version 400

			layout (binding=0) uniform sampler2D src;
			layout (location=0) uniform vec2 direction;

			in vec2 v_uv;
			layout (location=0) out vec4 fragColor;

			void main()
			{
				vec3 color = vec3(0.0);
				vec2 offset = 1.0 / textureSize(src, 0);

				for (float i = -5.0; i <= 5.0; i += 1.0)
				{
					color += texture(src, v_uv + direction * offset * i).rgb;
				}
				fragColor = vec4(color / 11.0, 1.0);
			})";

    static const char * vertexShaderSource =
        R"(#version 400

		layout (location=0) in vec2 a_vertex;
		out vec2 v_uv;

		void main()
		{
			v_uv = a_vertex.xy * 0.5 + 0.5;    
			gl_Position = vec4(a_vertex, 0.0, 1.0);
		}
		)";
}

struct texture_creater
{
    static yats::thread_group thread_constraints()
    {
        return yats::thread_group::main_thread();
    }

    yats::slot<std::unique_ptr<globjects::Texture>, "texture"_id> run(yats::slot<QImage, "image"_id> image)
    {
        auto texture = globjects::Texture::createDefault(GL_TEXTURE_2D);
        texture->image2D(0, GL_RGBA8, image->width(), image->height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image->bits());
        return std::move(texture);
    }
};

struct block_filter
{
    block_filter(float x, float y)
        : m_direction(x, y)
        , m_buffer(globjects::Buffer::create())
        , m_vao(globjects::VertexArray::create())
        , m_program(globjects::Program::create())
        , m_framebuffer(globjects::Framebuffer::create())
    {
        auto raw = std::array<glm::vec2, 4>{ { glm::vec2(+1.f,-1.f), glm::vec2(+1.f,+1.f), glm::vec2(-1.f,-1.f), glm::vec2(-1.f,+1.f) } };
        m_buffer->setData(raw, GL_STATIC_DRAW);

        auto binding = m_vao->binding(0);
        binding->setAttribute(0);
        binding->setBuffer(m_buffer.get(), 0, sizeof(glm::vec2));
        binding->setFormat(2, GL_FLOAT, GL_FALSE, 0);
        m_vao->enable(0);

        auto vertex_shader = globjects::Shader::create(GL_VERTEX_SHADER, globjects::Shader::sourceFromString(vertexShaderSource).get());
        auto fragment_shader = globjects::Shader::create(GL_FRAGMENT_SHADER, globjects::Shader::sourceFromString(fragmentShaderSource).get());
        m_program->attach(vertex_shader.get(), fragment_shader.get());
    }

    static yats::thread_group thread_constraints()
    {
        return yats::thread_group::main_thread();
    }

    yats::slot<std::unique_ptr<globjects::Texture>, "out_texture"_id> run(yats::slot<std::unique_ptr<globjects::Texture>, "in_texture"_id> texture)
    {
        auto result = globjects::Texture::createDefault(GL_TEXTURE_2D);
        auto width = (*texture)->getParameter(GL_TEXTURE_WIDTH);
        auto height = (*texture)->getParameter(GL_TEXTURE_HEIGHT);
        result->image2D(0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        m_framebuffer->attachTexture(GL_COLOR_ATTACHMENT0, result.get());

        glActiveTexture(GL_TEXTURE0);
        (*texture)->bind();
        m_program->use();
        m_program->setUniform(0, m_direction);

        m_vao->drawArrays(GL_TRIANGLE_STRIP, 0, 4);

        m_program->release();
        (*texture)->unbind();

        return std::move(result);
    }

protected:
    const glm::vec2 m_direction;

    std::unique_ptr<globjects::VertexArray> m_vao;
    std::unique_ptr<globjects::Buffer> m_buffer;
    std::unique_ptr<globjects::Program> m_program;
    std::unique_ptr<globjects::Framebuffer> m_framebuffer;
};

struct texture_writer
{
    static yats::thread_group thread_constraints()
    {
        return yats::thread_group::main_thread();
    }

    yats::slot<QImage, "image"_id> run(yats::slot<std::unique_ptr<globjects::Texture>, "texture"_id> texture)
    {
        auto width = (*texture)->getParameter(GL_TEXTURE_WIDTH);
        auto height = (*texture)->getParameter(GL_TEXTURE_HEIGHT);

        return QImage((*texture)->getImage(0, GL_RGBA, GL_UNSIGNED_BYTE).data(), width, height, QImage::Format::Format_RGBA8888);
    }
};

void window::run()
{
    globjects::init();

    std::cout << std::endl
        << "OpenGL Version:  " << glbinding::ContextInfo::version() << std::endl
        << "OpenGL Vendor:   " << glbinding::ContextInfo::vendor() << std::endl
        << "OpenGL Renderer: " << glbinding::ContextInfo::renderer() << std::endl << std::endl;
    globjects::DebugMessage::enable();

    yats::pipeline pipeline;

    auto image_loader = pipeline.add([](yats::slot<QString, "filename"_id> filename) -> yats::output_bundle<yats::slot<QImage, "image"_id>, yats::slot<QString, "name"_id>>
    {
        QFileInfo info(filename);
        return { QImage(filename), info.baseName() };
    });
    image_loader->mark_as_external<"filename"_id>();

    auto image_upload = pipeline.add<texture_creater>();
    image_loader->output<"image"_id>() >> image_upload->input<"image"_id>();
    image_loader->add_listener<"name"_id>([](QString name)
    {
        qDebug() << "Processing file" << name;
    });

    auto x_filter = pipeline.add<block_filter>(1.f, 0.f);
    auto y_filter = pipeline.add<block_filter>(0.f, 1.f);
    image_upload->output<"texture"_id>() >> x_filter->input<"in_texture"_id>();
    x_filter->output<"out_texture"_id>() >> y_filter->input<"in_texture"_id>();

    auto image_download = pipeline.add<texture_writer>();
    y_filter->output<"out_texture"_id>() >> image_download->input<"texture"_id>();

    auto image_writer = pipeline.add([](yats::slot<QImage, "image"_id> image, yats::slot<QString, "name"_id> name)
    {
        image->save(name + "_processed.png");
    });
    image_loader->output<"name"_id>() >> image_writer->input<"name"_id>();
    image_download->output<"image"_id>() >> image_writer->input<"image"_id>();

    yats::scheduler scheduler(std::move(pipeline));
    scheduler.run();
}