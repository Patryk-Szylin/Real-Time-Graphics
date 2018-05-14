#pragma once

#include <scene/scene_fwd.hpp>
#include <tygra/WindowViewDelegate.hpp>
#define TGL_TARGET_GL_4_4
#include <tgl/tgl.h>
#include <glm/glm.hpp>


#include <vector>
#include <memory>
#include <map>

class MyView : public tygra::WindowViewDelegate
{
public:

	enum TEXTURE_TYPE
	{
		GBUFFER_POSITION_TEXTURE,
		GBUFFER_NORMAL_TEXTURE,
		GBUFFER_COLOUR_TEXTURE,
		SHADOW_MAP,
		FINAL_COLOUR,
		NUM_TEXTURES
	};



	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
	};

	struct Program
	{
		GLuint gBuffer;
		GLuint directionalLight;
		GLuint pointLight;
		GLuint shadowDepth;
		GLuint spotLight;
		GLuint postProcess;
	};

	struct VertexBuffer
	{
		GLuint VBO;
		GLuint VAO;
		GLuint EBO;

		GLuint instance_VBO;
	};

	struct FrameBuffer
	{
		GLuint fbo;
		GLuint depth_rbo;
		GLuint colour_rbo;
		GLuint textures[NUM_TEXTURES];
	};

	struct PerPointLightUniforms
	{
		glm::mat4 combined_xform;
		glm::vec3 light_position;
		float light_range;
		glm::vec3 light_intensity;
	};

	struct PerSpotLightUniforms
	{
		glm::mat4 combined_xform;

		glm::vec3 light_position;
		float light_range;
		glm::vec3 light_intensity;
		float light_angle;
		glm::vec3 light_direction;
	};

	struct PerModelUniforms
	{
		glm::mat4 projection_xform;
		glm::mat4 view_xform;
	};

	struct Mesh
	{
		GLuint base_index;
		GLuint base_vertex;
		unsigned int element_count;
		unsigned int instance_count;
		unsigned int base_xform;
		unsigned int mesh_id;

		Mesh()
		{
			base_index = 0;
			base_vertex = 0;
			element_count = 0;
			instance_count = 0;
			base_xform = 0;
			mesh_id = 0;
		}
	};

	struct LightMesh
	{
		GLuint vertex_vbo{ 0 };
		GLuint element_vbo{ 0 };
		GLuint vao{ 0 };
		int element_count{ 0 };
	};

	struct Sponza_Data
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Mesh> entries;					// Unique mesh entries
		std::vector<glm::mat4> modelMatrices;
	};

	MyView();

	~MyView();

	void setScene(const scene::Context * scene);

private:

	void windowViewWillStart(tygra::Window * window) override;

	void windowViewDidReset(tygra::Window * window,
		int width,
		int height) override;

	void windowViewDidStop(tygra::Window * window) override;
	void windowViewRender(tygra::Window * window) override;

	// My functions
	GLuint createProgram(std::string vertex_shader, std::string fragment_shader);
	void MeshGL();
	void setupSponzaBuffers();

	// Deferred rendering functions
	void GBufferPass(const glm::mat4& projection, const glm::mat4& view);
	void DirectionalLightPass(const glm::vec3& camera_position);
	void PointLightPass(const glm::mat4& projection, const glm::mat4& view);
	void ShadowDepthPass(const glm::mat4& lightSpace_xform);
	void SpotLightPass(const glm::mat4& projection, const glm::mat4& view, const glm::mat4& lightSpace_xform);
	void StencilPass();		// TODO
	void PostProcess(int textureWidth, int textureHeight);


	std::map<scene::MeshId, Mesh> mesh_map;
	std::map<scene::MeshId, scene::MaterialId> material_map;

	// Structure objects
	FrameBuffer m_gBuffer;
	FrameBuffer m_lBuffer;
	FrameBuffer m_sBuffer;
	VertexBuffer m_sponza;
	Program _program;
	Sponza_Data m_sponzaData;
	LightMesh m_lightQuad;
	LightMesh m_lightSphere;
	LightMesh m_lightCone;

	// UBO objects
	PerModelUniforms m_ubo_matrices;
	PerPointLightUniforms m_ubo_pointLight;
	PerSpotLightUniforms m_ubo_spotLight;

	// Globals
	int _width;
	int _height;
	GLuint _ubo_pointLight;
	GLuint _ubo_spotLight;
	glm::vec3 _camera_position;
	glm::vec3 _camera_direction;


	const scene::Context * scene_;



};
