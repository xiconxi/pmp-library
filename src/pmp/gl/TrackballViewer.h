//=============================================================================
// Copyright (C) 2011-2019 The pmp-library developers
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//=============================================================================
#pragma once
//=============================================================================

#include <pmp/gl/Window.h>
#include <pmp/MatVec.h>

#include <string>
#include <vector>

//=============================================================================

namespace pmp {

//=============================================================================

//! \addtogroup gl gl
//! @{

//! A simple GLFW viewer with trackball user interface
class TrackballViewer : public Window
{

public: //------------------------------------------------------ public methods
    //! constructor
    TrackballViewer(const char* title, int width, int height,
                    bool showgui = true);

    //! destructor
    virtual ~TrackballViewer();

    //! define the center and radius of the scene/
    //! used for trackball rotation
    void set_scene(const vec3& center, float radius);

    //! adjust camera such that the whole scene (defined by set_scene()) is visible
    void view_all();

protected: //----------------------------------- callbacks as member functions
    //! this function is called when the scene has to be rendered. it
    //! clears the buffers, calls the draw() method, and performs buffer swap
    virtual void display(void) override;

    //! this function handles keyboard events
    virtual void keyboard(int key, int code, int action, int mods) override;

    //! this function handles mouse button events
    virtual void mouse(int button, int action, int mods) override;

    //! this function handles mouse motion (passive/active position)
    virtual void motion(double xpos, double ypos) override;

    //! this function handles mouse scroll events
    virtual void scroll(double xoffset, double yoffset) override;

    //! this function is called if the window is resized
    virtual void resize(int width, int height) override;

protected: //------------------------------------------- handling of draw modes
    //! reset the list of draw modes
    void clear_draw_modes();

    //! add a draw mode
    unsigned int add_draw_mode(const std::string& drawMode);

    //! activate a draw mode
    void set_draw_mode(const std::string& drawMode);

protected: //-------------------------------------------------------- rendering
    //! initialize all OpenGL states
    virtual void init();

    //! this function is responsible for rendering the scene
    virtual void draw(const std::string& drawMode) = 0;

protected: //-------------------------------------------- trackball interaction
    //! turn a mouse event into a rotation around the scene center. calls rotate().
    void rotation(int x, int y);

    //! turn a mouse event into a translation in the view plane. calls translate().
    void translation(int x, int y);

    //! turn a mouse event into a zoom, i.e., translation in z-direction. calls translate().
    void zoom(int x, int y);

    //! get 3D position under the mouse cursor
    bool pick(int x, int y, vec3& result);

    //! fly toward the position under the mouse cursor and set rotation center to it
    void fly_to(int x, int y);

    //! translate the scene and update modelview matrix
    void translate(const vec3& trans);

    //! rotate the scene (around its center) and update modelview matrix
    void rotate(const vec3& axis, float angle);

    //! virtual trackball: map 2D screen point to unit sphere. used by rotate().
    bool map_to_sphere(const ivec2& point, vec3& result);

protected: //----------------------------------------------------- private data
    //! draw modes
    unsigned int draw_mode_;
    unsigned int n_draw_modes_;
    std::vector<std::string> draw_mode_names_;

    //! scene position and dimension
    vec3 center_;
    float radius_;

    //! projection parameters
    float near_, far_, fovy_;

    //! OpenGL matrices
    mat4 projection_matrix_;
    mat4 modelview_matrix_;

    //! trackball helpers
    ivec2 last_point_2d_;
    vec3 last_point_3d_;
    bool last_point_ok_;
    bool button_down_[7];
    int modifiers_;
    int wheel_pos_;
};

//=============================================================================
//! @}
//=============================================================================
} // namespace pmp
//=============================================================================
