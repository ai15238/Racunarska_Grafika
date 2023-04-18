//
// Created by matf-rg on 30.10.20..
//

#ifndef PROJECT_BASE_TEXTURE2D_H
#define PROJECT_BASE_TEXTURE2D_H

#include <glad/glad.h>
#include <stb_image.h>
#include <rg/Error.h>


class Texture2D {
public:
    Texture2D(std::string pathToImg, GLenum sampling, GLenum filtering, GLenum rgb) {
        unsigned int tex0;
        glGenTextures(1, &tex0); //generisemo tekstura objekat

        glBindTexture(GL_TEXTURE_2D, tex0);

        //wwrap
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sampling);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, sampling);

        //filter
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering);

        //load img
        int width, height, nChannel;
        stbi_set_flip_vertically_on_load(true);

        ASSERT(!pathToImg.empty(), "Check path to img! ");
        data = stbi_load(pathToImg.c_str(), &width, &height, &nChannel, 0);

        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, rgb, width, height, 0, rgb, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            std::cerr << "Failed to load img";
            delete_tex();
        }

        m_Id = tex0;

    }

    ~Texture2D() {
        delete_tex();
    }

    void delete_tex() {
        stbi_image_free(data);
    }

    void active(GLenum e) {
        glActiveTexture(e);
    }

    void bind() {
        glBindTexture(GL_TEXTURE_2D, m_Id);
    }

private:
    unsigned int m_Id;
    unsigned char *data;
};

#endif //PROJECT_BASE_TEXTURE2D_H