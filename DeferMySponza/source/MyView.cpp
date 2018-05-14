#include "MyView.hpp"

#include <scene/scene.hpp>
#include <tygra/FileHelper.hpp>
#include <tsl/shapes.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cassert>


MyView::MyView()
{
}

MyView::~MyView() {
}

void MyView::setScene(const scene::Context * scene)
{
	scene_ = scene;
}

void MyView::windowViewWillStart(tygra::Window * window)
{
	assert(scene_ != nullptr);

	_program.gBuffer = createProgram("resource:///gbuffer_vs.glsl", "resource:///gbuffer_fs.glsl");
	_program.directionalLight = createProgram("resource:///directional_light_vs.glsl", "resource:///directional_light_fs.glsl");
	_program.pointLight = createProgram("resource:///point_light_vs.glsl", "resource:///point_light_fs.glsl");
	_program.shadowDepth = createProgram("resource:///shadow_depth_vs.glsl", "resource:///shadow_depth_fs.glsl");
	_program.spotLight = createProgram("resource:///spot_light_vs.glsl", "resource:///spot_light_fs.glsl");
	_program.postProcess = createProgram("resource:///post_process_vs.glsl", "resource:///post_process_fs.glsl");

	// Build meshes
	MeshGL();

	// Build sponza's vertex buffers
	glGenBuffers(1, &m_sponza.VBO);
	glGenBuffers(1, &m_sponza.EBO);
	glGenVertexArrays(1, &m_sponza.VAO);

	setupSponzaBuffers();

	// Generate objects here
	glGenFramebuffers(1, &m_gBuffer.fbo);
	glGenRenderbuffers(1, &m_gBuffer.depth_rbo);

	glGenFramebuffers(1, &m_sBuffer.fbo);
	glGenRenderbuffers(1, &m_sBuffer.depth_rbo);

	glGenFramebuffers(1, &m_lBuffer.fbo);
	glGenRenderbuffers(1, &m_lBuffer.colour_rbo);

	glGenTextures(1, &m_gBuffer.textures[GBUFFER_POSITION_TEXTURE]);
	glGenTextures(1, &m_gBuffer.textures[GBUFFER_NORMAL_TEXTURE]);
	glGenTextures(1, &m_gBuffer.textures[GBUFFER_COLOUR_TEXTURE]);

	glGenTextures(1, &m_sBuffer.textures[SHADOW_MAP]);
	glGenTextures(1, &m_lBuffer.textures[FINAL_COLOUR]);


	// Create light meshes here
	// Screen quad
	std::vector<glm::vec2> vertices(4);
	vertices[0] = glm::vec2(-1, -1);
	vertices[1] = glm::vec2(1, -1);
	vertices[2] = glm::vec2(1, 1);
	vertices[3] = glm::vec2(-1, 1);

	glGenBuffers(1, &m_lightQuad.vertex_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_lightQuad.vertex_vbo);
	glBufferData(GL_ARRAY_BUFFER,
		vertices.size() * sizeof(glm::vec2),
		vertices.data(),
		GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &m_lightQuad.vao);
	glBindVertexArray(m_lightQuad.vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_lightQuad.vertex_vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
		sizeof(glm::vec2), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Point light
	tsl::IndexedMeshPtr sphere_mesh = tsl::createSpherePtr(1.0f, 12);
	sphere_mesh = tsl::cloneIndexedMeshAsTriangleListPtr(sphere_mesh.get());


	m_lightSphere.element_count = sphere_mesh->indexCount();
	glGenBuffers(1, &m_lightSphere.vertex_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_lightSphere.vertex_vbo);
	glBufferData(GL_ARRAY_BUFFER, sphere_mesh->vertexCount() * sizeof(glm::vec3), sphere_mesh->positionArray(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m_lightSphere.element_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_lightSphere.element_vbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere_mesh->indexCount() * sizeof(unsigned int), sphere_mesh->indexArray(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &m_lightSphere.vao);
	glBindVertexArray(m_lightSphere.vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_lightSphere.vertex_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_lightSphere.element_vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Spot light
	tsl::IndexedMeshPtr cone_mesh = tsl::createConePtr(1.f, 1.f, 12);
	cone_mesh = tsl::cloneIndexedMeshAsTriangleListPtr(cone_mesh.get());
	m_lightCone.element_count = cone_mesh->indexCount();


	glGenBuffers(1, &m_lightCone.vertex_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_lightCone.vertex_vbo);
	glBufferData(GL_ARRAY_BUFFER,
		cone_mesh->vertexCount() * sizeof(glm::vec3),
		cone_mesh->positionArray(),
		GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m_lightCone.element_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_lightCone.element_vbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		cone_mesh->indexCount() * sizeof(unsigned int),
		cone_mesh->indexArray(),
		GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &m_lightCone.vao);
	glBindVertexArray(m_lightCone.vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_lightCone.element_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_lightCone.vertex_vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
		sizeof(glm::vec3), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Setup ubos
	glGenBuffers(1, &_ubo_pointLight);
	glBindBuffer(GL_UNIFORM_BUFFER, _ubo_pointLight);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PerPointLightUniforms), nullptr, GL_STREAM_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, _ubo_pointLight);
	glUniformBlockBinding(_program.pointLight, glGetUniformBlockIndex(_program.pointLight, "PerLightUniforms"), 1);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glGenBuffers(1, &_ubo_spotLight);
	glBindBuffer(GL_UNIFORM_BUFFER, _ubo_spotLight);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PerSpotLightUniforms), nullptr, GL_STREAM_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, _ubo_spotLight);
	glUniformBlockBinding(_program.spotLight, glGetUniformBlockIndex(_program.spotLight, "PerLightUniforms"), 2);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);







	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);



}

void MyView::windowViewDidReset(tygra::Window * window,
	int width,
	int height)
{
	glViewport(0, 0, width, height);

	// Setup GBUFFER attachments
	glBindTexture(GL_TEXTURE_RECTANGLE, m_gBuffer.textures[GBUFFER_POSITION_TEXTURE]);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_RECTANGLE, m_gBuffer.textures[GBUFFER_NORMAL_TEXTURE]);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_RECTANGLE, m_gBuffer.textures[GBUFFER_COLOUR_TEXTURE]);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindRenderbuffer(GL_RENDERBUFFER, m_gBuffer.depth_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	// Bind Gbuffer
	GLenum gbuffer_status = 0;
	glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer.fbo);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_gBuffer.depth_rbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, m_gBuffer.textures[GBUFFER_POSITION_TEXTURE], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_RECTANGLE, m_gBuffer.textures[GBUFFER_NORMAL_TEXTURE], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_RECTANGLE, m_gBuffer.textures[GBUFFER_COLOUR_TEXTURE], 0);

	gbuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (gbuffer_status != GL_FRAMEBUFFER_COMPLETE) {
		tglDebugMessage(GL_DEBUG_SEVERITY_HIGH_ARB, "GBUFFER is framebuffer not complete!");
	}

	GLenum gbufs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, gbufs);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);



	// Setting up LBUFFER as a colour buffer
	glBindRenderbuffer(GL_RENDERBUFFER, m_lBuffer.colour_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, m_lBuffer.textures[FINAL_COLOUR]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);



	GLuint lbuffer_status = 0;
	glBindFramebuffer(GL_FRAMEBUFFER, m_lBuffer.fbo);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_gBuffer.depth_rbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_lBuffer.textures[FINAL_COLOUR], 0);


	lbuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (lbuffer_status != GL_FRAMEBUFFER_COMPLETE)
		tglDebugMessage(GL_DEBUG_SEVERITY_HIGH_ARB, "LBUFFER framebuffer is not complete!");

	GLenum lbufs[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, lbufs);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Shadow framebuffer
	glBindTexture(GL_TEXTURE_RECTANGLE, m_sBuffer.textures[SHADOW_MAP]);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_RECTANGLE, GL_TEXTURE_BORDER_COLOR, borderColor);



	GLenum shadowBuffer_status = 0;
	glBindFramebuffer(GL_FRAMEBUFFER, m_sBuffer.fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_RECTANGLE, m_sBuffer.textures[SHADOW_MAP], 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	shadowBuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (lbuffer_status != GL_FRAMEBUFFER_COMPLETE)
		tglDebugMessage(GL_DEBUG_SEVERITY_HIGH_ARB, "SHADOWBUFFER framebuffer is not complete!");
}

void MyView::windowViewDidStop(tygra::Window * window)
{
	// Programs
	glDeleteProgram(_program.gBuffer);
	glDeleteProgram(_program.directionalLight);
	glDeleteProgram(_program.pointLight);
	glDeleteProgram(_program.postProcess);
	glDeleteProgram(_program.shadowDepth);
	glDeleteProgram(_program.spotLight);

	//// Buffers
	//glDeleteFramebuffers(1, &m_gBuffer.fbo);
	//glDeleteFramebuffers(1, &m_lBuffer.fbo);
	//glDeleteFramebuffers(1, &m_sBuffer.fbo);

	//glDeleteRenderbuffers(1, &m_gBuffer.depth_rbo);

	//glDeleteBuffers(1, &m_lightQuad.element_vbo);
	//glDeleteBuffers(1, &m_lightQuad.vertex_vbo);
	//glDeleteBuffers(1, &m_lightQuad.vao);

	//glDeleteBuffers(1, &m_lightCone.element_vbo);
	//glDeleteBuffers(1, &m_lightCone.vertex_vbo);
	//glDeleteBuffers(1, &m_lightCone.vao);

	//glDeleteBuffers(1, &m_lightSphere.element_vbo);
	//glDeleteBuffers(1, &m_lightSphere.vertex_vbo);
	//glDeleteBuffers(1, &m_lightSphere.vao);

	//// Textures
	//glDeleteTextures(NUM_TEXTURES, m_gBuffer.textures);
	//glDeleteTextures(NUM_TEXTURES, m_sBuffer.textures);
	//glDeleteTextures(NUM_TEXTURES, m_lBuffer.textures);
}

void MyView::windowViewRender(tygra::Window * window)
{
	assert(scene_ != nullptr);

	GLint viewportSize[4];
	glGetIntegerv(GL_VIEWPORT, viewportSize);
	const float aspectRatio = viewportSize[2] / (float)viewportSize[3];
	_width = viewportSize[2];
	_height = viewportSize[3];

	glViewport(0, 0, _width, _height);


	scene::Camera camera = scene_->getCamera();
	_camera_position = (const glm::vec3&)camera.getPosition();
	_camera_direction = (const glm::vec3&)camera.getDirection();

	// Matrices
	glm::mat4 projection_xform, view_xform;

	projection_xform = glm::perspective(glm::radians(camera.getVerticalFieldOfViewInDegrees()), aspectRatio, camera.getNearPlaneDistance(), camera.getFarPlaneDistance());
	view_xform = glm::lookAt(_camera_position, _camera_direction + _camera_position, (const glm::vec3 &)scene_->getUpDirection());



	///////////////////////////////////////////////
	// Setup Shadow map matrices
	///////////////////////////////////////////////

	GLfloat near_plane = 0.1f;
	GLfloat far_plane = 500.0f;

	const auto& allSpotLights = scene_->getAllSpotLights();
	glm::vec3 light_position = (const glm::vec3&)allSpotLights[1].getPosition();
	glm::vec3 light_direction = (const glm::vec3&)allSpotLights[1].getDirection();
	const float light_aspectRatio = 1024.0f / 1024.0f;

	glm::mat4 lightProjection = glm::perspective(glm::radians(60.0f), light_aspectRatio, near_plane, far_plane);
	glm::mat4 lightView = glm::lookAt(light_position, light_position + light_direction, glm::vec3(0, 1, 0));
	glm::mat4 lightSpace_xform = lightProjection * lightView;


	///////////////////////////////////////////////
	// Deferred Passes
	///////////////////////////////////////////////

	ShadowDepthPass(lightSpace_xform);

	// Reset viewport back to normal after generating shadow map
	glViewport(0, 0, _width, _height);


	GBufferPass(projection_xform, view_xform);
	DirectionalLightPass(_camera_position);
	PointLightPass(projection_xform, view_xform);

	glEnable(GL_STENCIL_TEST);

	SpotLightPass(projection_xform, view_xform, lightSpace_xform);
	PostProcess(_width, _height);



}

void MyView::StencilPass()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);

	glDisable(GL_CULL_FACE);
	glClear(GL_STENCIL_BUFFER_BIT);



	glStencilFunc(GL_ALWAYS, 0, 0);
	glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
	glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
}



// DEFERRED RENDERING FUNCTIONS
void MyView::ShadowDepthPass(const glm::mat4& lightSpace_xform)
{
	glEnable(GL_DEPTH_TEST);

	glUseProgram(_program.shadowDepth);

	glUniformMatrix4fv(glGetUniformLocation(_program.shadowDepth, "lightSpace_xform"), 1, GL_FALSE, glm::value_ptr(lightSpace_xform));

	glViewport(0, 0, 1024, 1024);

	glBindFramebuffer(GL_FRAMEBUFFER, m_sBuffer.fbo);
	glClear(GL_DEPTH_BUFFER_BIT);

	const auto& allInstances = scene_->getAllInstances();

	glBindVertexArray(m_sponza.VAO);

	for (GLuint i = 0; i < m_sponzaData.entries.size(); i++)
	{
		auto& mesh = allInstances[i];

		unsigned int offset = (sizeof(unsigned int) * m_sponzaData.entries[i].base_index);

		glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES,
			m_sponzaData.entries[i].element_count,
			GL_UNSIGNED_INT,
			(void*)offset,
			m_sponzaData.entries[i].instance_count,
			m_sponzaData.entries[i].base_vertex,
			m_sponzaData.entries[i].base_xform);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void MyView::GBufferPass(const glm::mat4& projection, const glm::mat4& view)
{
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 0x00, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	// Bind gbuffer for writing
	glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer.fbo);

	// Clear buffers
	glClearDepth(1.0);
	glClearStencil(128);
	glClearColor(0.0f, 0.0f, 0.25f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glUseProgram(_program.gBuffer);

	glBindVertexArray(m_sponza.VAO);

	glClearStencil(0);
	const auto& allInstances = scene_->getAllInstances();

	unsigned int obj_ID = 0;
	std::vector<glm::mat4> animated_xform;
	for (size_t i = 0; i < allInstances.size(); i++)
	{
		if (allInstances[i].getMeshId() == 300)		// 300 Is the vases which have animated matrices
		{
			animated_xform.push_back(glm::translate(glm::mat4((const glm::mat4x3 &)allInstances[i].getTransformationMatrix()), glm::vec3(0, 0, 0)));
			obj_ID++;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_sponza.instance_VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::mat4) * 4, animated_xform.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);



	glUniformMatrix4fv(glGetUniformLocation(_program.gBuffer, "projection_xform"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(glGetUniformLocation(_program.gBuffer, "view_xform"), 1, GL_FALSE, glm::value_ptr(view));


	for (GLuint i = 0; i < m_sponzaData.entries.size(); i++)
	{
		auto& mesh = allInstances[i];

		const scene::MaterialId mat_id = mesh.getMaterialId();
		const scene::Material material = scene_->getMaterialById(material_map[m_sponzaData.entries[i].mesh_id]);

		// Fill the albedoTexture
		glUniform3fv(glGetUniformLocation(_program.gBuffer, "albedo_color"), 1, glm::value_ptr((const glm::vec3&)(material.getDiffuseColour())));

		glUniform1f(glGetUniformLocation(_program.gBuffer, "albedo_shininess"), material.getShininess());



		//unsigned int temp = m_Entries[i].instance_count;
		unsigned int offset = (sizeof(unsigned int) * m_sponzaData.entries[i].base_index);


		glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES,
			m_sponzaData.entries[i].element_count,
			GL_UNSIGNED_INT,
			(void*)offset,
			m_sponzaData.entries[i].instance_count,
			m_sponzaData.entries[i].base_vertex,
			m_sponzaData.entries[i].base_xform);
	}

	// Reset OpenGL Status
	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_STENCIL_TEST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void MyView::DirectionalLightPass(const glm::vec3& camera_position)
{
	glDepthMask(GL_FALSE);

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 0, ~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);


	// Bind colour buffer for writing 
	glBindFramebuffer(GL_FRAMEBUFFER, m_lBuffer.fbo);

	// Clear buffers
	glClearColor(0.f, 0.f, 0.25f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(_program.directionalLight);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, m_gBuffer.textures[GBUFFER_POSITION_TEXTURE]);
	glUniform1i(glGetUniformLocation(_program.directionalLight, "sampler_gPosition"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE, m_gBuffer.textures[GBUFFER_NORMAL_TEXTURE]);
	glUniform1i(glGetUniformLocation(_program.directionalLight, "sampler_gNormal"), 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_RECTANGLE, m_gBuffer.textures[GBUFFER_COLOUR_TEXTURE]);
	glUniform1i(glGetUniformLocation(_program.directionalLight, "sampler_gAlbedo"), 2);

	glUniform3fv(glGetUniformLocation(_program.directionalLight, "camera_positio"), 1, glm::value_ptr(camera_position));

	glBindVertexArray(m_lightQuad.vao);
	glm::vec3 ambientlight_intensity = (glm::vec3 &)scene_->getAmbientLightIntensity();
	glUniform3fv(glGetUniformLocation(_program.directionalLight, "ambient_intensity"), 1, glm::value_ptr(ambientlight_intensity));


	// Draw light quad
	auto& allDirectionalLights = scene_->getAllDirectionalLights();
	for (size_t i = 0; i < allDirectionalLights.size(); i++)
	{
		glm::vec3 light_direction = (glm::vec3 &)allDirectionalLights[i].getDirection();
		glm::vec3 light_intensity = (glm::vec3 &)allDirectionalLights[i].getIntensity();

		glUniform3fv(glGetUniformLocation(_program.directionalLight, ("light_direction[" + std::to_string(i) + "]").c_str()), 1, glm::value_ptr(light_direction));
		glUniform3fv(glGetUniformLocation(_program.directionalLight, ("light_intensity[" + std::to_string(i) + "]").c_str()), 1, glm::value_ptr(light_intensity));

		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}

	glDisable(GL_STENCIL_TEST);
}

void MyView::PointLightPass(const glm::mat4& projection, const glm::mat4& view)
{
	glDepthMask(GL_FALSE);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	glUseProgram(_program.pointLight);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, m_gBuffer.textures[GBUFFER_POSITION_TEXTURE]);
	glUniform1i(glGetUniformLocation(_program.pointLight, "sampler_gPosition"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE, m_gBuffer.textures[GBUFFER_NORMAL_TEXTURE]);
	glUniform1i(glGetUniformLocation(_program.pointLight, "sampler_gNormal"), 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_RECTANGLE, m_gBuffer.textures[GBUFFER_COLOUR_TEXTURE]);
	glUniform1i(glGetUniformLocation(_program.pointLight, "sampler_gAlbedo"), 2);

	glBindVertexArray(m_lightSphere.vao);

	auto& allPointLights = scene_->getAllPointLights();

	for (size_t i = 0; i < allPointLights.size(); i++)
	{
		glm::mat4 model_xform;
		model_xform = glm::translate(model_xform, (glm::vec3 &)allPointLights[i].getPosition());
		model_xform = glm::scale(model_xform, glm::vec3(allPointLights[i].getRange()));

		glm::mat4 combined_xform = projection * view * model_xform;
		m_ubo_pointLight.combined_xform = combined_xform;


		m_ubo_pointLight.light_position = (glm::vec3 &)allPointLights[i].getPosition();
		m_ubo_pointLight.light_intensity = (glm::vec3 &)allPointLights[i].getIntensity();
		m_ubo_pointLight.light_range = allPointLights[i].getRange();


		glBindBuffer(GL_UNIFORM_BUFFER, _ubo_pointLight);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PerPointLightUniforms), &m_ubo_pointLight);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glDrawElements(GL_TRIANGLES, m_lightSphere.element_count, GL_UNSIGNED_INT, 0);


	}

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_STENCIL_TEST);

}

void MyView::SpotLightPass(const glm::mat4& projection, const glm::mat4& view, const glm::mat4& lightSpace_xform)
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);

	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);


	glStencilFunc(GL_EQUAL, 0, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	glUseProgram(_program.spotLight);

	glUniformMatrix4fv(glGetUniformLocation(_program.spotLight, "lightSpace_xform"), 1, GL_FALSE, glm::value_ptr(lightSpace_xform));

	// gBufferTextures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, m_gBuffer.textures[GBUFFER_POSITION_TEXTURE]);
	glUniform1i(glGetUniformLocation(_program.spotLight, "sampler_gPosition"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE, m_gBuffer.textures[GBUFFER_NORMAL_TEXTURE]);
	glUniform1i(glGetUniformLocation(_program.spotLight, "sampler_gNormal"), 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_RECTANGLE, m_gBuffer.textures[GBUFFER_COLOUR_TEXTURE]);
	glUniform1i(glGetUniformLocation(_program.spotLight, "sampler_gAlbedo"), 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_RECTANGLE, m_sBuffer.textures[SHADOW_MAP]);
	glUniform1i(glGetUniformLocation(_program.spotLight, "sampler_gAlbedo"), 3);

	glm::vec3 ambientlight_intensity = (glm::vec3 &)scene_->getAmbientLightIntensity();
	glUniform3fv(glGetUniformLocation(_program.spotLight, "ambient_intensity"), 1, glm::value_ptr(ambientlight_intensity));


	glBindVertexArray(m_lightCone.vao);

	auto& allSpotLights = scene_->getAllSpotLights();
	for (size_t i = 0; i < allSpotLights.size(); i++)
	{
		glm::mat4 model_xform;

		bool castShadow = allSpotLights[i].getCastShadow() == true ? true : false;

		glUniform1i(glGetUniformLocation(_program.spotLight, "castShadow"), castShadow);

		model_xform = glm::translate(model_xform, glm::vec3(0, 0, -1));
		glm::vec3 scaling_factors;
		glm::mat4 scale_xform;
		scaling_factors.z = allSpotLights[i].getRange();


		// Calculate cone's base area
		float half_angle = allSpotLights[i].getConeAngleDegrees() / 2;				// 30 : 45
		float tangentVal = glm::tan(glm::degrees(half_angle));						// 0.5735 : 1.61977

		float tan = half_angle == 45 ? 1 : tangentVal;

		float radius = tan * scaling_factors.z;

		scaling_factors.y = scaling_factors.x = radius;

		scale_xform = glm::scale(scale_xform, scaling_factors);
		model_xform = glm::inverse(glm::lookAt((const glm::vec3 &)allSpotLights[i].getPosition(),
			(const glm::vec3 &)allSpotLights[i].getDirection() +
			(const glm::vec3 &)allSpotLights[i].getPosition(),
			(const glm::vec3 &)scene_->getUpDirection())) * scale_xform * model_xform;

		glm::mat4 combined_xform = projection * view * model_xform;
		m_ubo_spotLight.combined_xform = combined_xform;


		m_ubo_spotLight.light_position = (glm::vec3 &)allSpotLights[i].getPosition();
		m_ubo_spotLight.light_intensity = (glm::vec3 &)allSpotLights[i].getIntensity();
		m_ubo_spotLight.light_direction = (glm::vec3 &)allSpotLights[i].getDirection();
		m_ubo_spotLight.light_range = allSpotLights[i].getRange();
		m_ubo_spotLight.light_angle = allSpotLights[i].getConeAngleDegrees();

		glBindBuffer(GL_UNIFORM_BUFFER, _ubo_spotLight);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PerSpotLightUniforms), &m_ubo_spotLight);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glDrawElements(GL_TRIANGLES, m_lightCone.element_count, GL_UNSIGNED_INT, 0);
	}

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_STENCIL_TEST);

}


void MyView::PostProcess(int textureWidth, int textureHeight)
{
	glUseProgram(_program.postProcess);


	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_lBuffer.fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_lBuffer.textures[FINAL_COLOUR]);

	glUniform1i(glGetUniformLocation(_program.postProcess, "screen_tex"), 0);
	
	glm::vec2 resolution = glm::vec2(textureWidth, textureHeight);

	glUniform2fv(glGetUniformLocation(_program.postProcess, "resolution"), 1, glm::value_ptr(resolution));

	glBindVertexArray(m_lightQuad.vao);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

}


GLuint MyView::createProgram(std::string vertex_shader, std::string fragment_shader)
{
	GLuint program_vs, program_fs;
	GLint compile_status = 0;

	GLuint programID;

	std::string program_vs_string = tygra::createStringFromFile(vertex_shader);
	const char* program_vs_code = program_vs_string.c_str();
	program_vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(program_vs, 1, &program_vs_code, NULL);
	glCompileShader(program_vs);

	glGetShaderiv(program_vs, GL_COMPILE_STATUS, &compile_status);
	if (compile_status != GL_TRUE)
	{
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetShaderInfoLog(program_vs, string_length, NULL, log);
		std::cerr << log << std::endl;
	}

	std::string program_fs_string = tygra::createStringFromFile(fragment_shader);
	const char* program_fs_code = program_fs_string.c_str();
	program_fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(program_fs, 1, &program_fs_code, NULL);
	glCompileShader(program_fs);

	glGetShaderiv(program_fs, GL_COMPILE_STATUS, &compile_status);
	if (compile_status != GL_TRUE)
	{
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetShaderInfoLog(program_fs, string_length, NULL, log);
		std::cerr << log << std::endl;
	}

	// Use function's program variable to create program
	programID = glCreateProgram();
	glAttachShader(programID, program_vs);
	glAttachShader(programID, program_fs);
	glLinkProgram(programID);

	GLint link_status = 0;
	glGetProgramiv(programID, GL_LINK_STATUS, &link_status);
	if (link_status != GL_TRUE) {
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetProgramInfoLog(programID, string_length, NULL, log);
		std::cerr << log << std::endl;
	}

	return programID;


}

void MyView::MeshGL()
{
	// Fill data
	scene::GeometryBuilder builder;

	const auto& allMeshes = builder.getAllMeshes();
	const auto& allInstances = scene_->getAllInstances();

	for (size_t i = 0; i < allMeshes.size(); i++)
	{
		const scene::Mesh mesh = allMeshes[i];

		Mesh new_mesh;

		const auto& mesh_positions = mesh.getPositionArray();
		const auto& mesh_normals = mesh.getNormalArray();
		const auto& mesh_elements = mesh.getElementArray();
		const auto& mesh_texCoords = mesh.getTextureCoordinateArray();


		new_mesh.element_count = mesh_elements.size();
		new_mesh.mesh_id = mesh.getId();
		new_mesh.base_index = m_sponzaData.indices.size();
		new_mesh.base_vertex = m_sponzaData.vertices.size();
		new_mesh.base_xform = m_sponzaData.modelMatrices.size();

		std::vector<Vertex> mesh_vertexData(mesh_positions.size());

		for (size_t j = 0; j < mesh_positions.size(); j++)
		{
			Vertex currentVertex;
			currentVertex.Position = (glm::vec3&)mesh_positions[j];
			currentVertex.Normal = (glm::vec3&)mesh_normals[j];

			if (mesh_texCoords.size() == 0)
			{
				currentVertex.TexCoords = glm::vec2(0, 0);
			}
			else
			{
				currentVertex.TexCoords = (glm::vec2&)mesh_texCoords[j];
			}

			mesh_vertexData[j] = currentVertex;

		}


		int instanceIndex;
		for (size_t j = 0; j < allInstances.size(); j++)
		{
			auto& currentInstance = allInstances[j];
			//modelMatrices = new glm::mat4[allInstances.size()];
			glm::mat4 model_xform;

			if (currentInstance.getMeshId() == mesh.getId())
			{
				instanceIndex = j;
				model_xform = glm::translate(glm::mat4((const glm::mat4x3 &) currentInstance.getTransformationMatrix()), glm::vec3(0, 0, 0));
				m_sponzaData.modelMatrices.push_back(model_xform);
				new_mesh.instance_count++;
			}
		}

		m_sponzaData.vertices.insert(m_sponzaData.vertices.end(), mesh_vertexData.begin(), mesh_vertexData.end());
		m_sponzaData.indices.insert(m_sponzaData.indices.end(), mesh_elements.begin(), mesh_elements.end());
		m_sponzaData.entries.push_back(new_mesh);


		mesh_map[mesh.getId()] = new_mesh;
		material_map[mesh.getId()] = allInstances[instanceIndex].getMaterialId();
	}

}

void MyView::setupSponzaBuffers()
{
	// Fill Buffers

	glBindBuffer(GL_ARRAY_BUFFER, m_sponza.VBO);
	glBufferData(GL_ARRAY_BUFFER, m_sponzaData.vertices.size() * sizeof(Vertex), m_sponzaData.vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_sponza.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_sponzaData.indices.size() * sizeof(unsigned int), m_sponzaData.indices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


	glBindVertexArray(m_sponza.VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_sponza.EBO);

	glBindBuffer(GL_ARRAY_BUFFER, m_sponza.VBO);
	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), TGL_BUFFER_OFFSET(0));
	// Normals attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), TGL_BUFFER_OFFSET(sizeof(float) * 3));
	// TexCoords attribute
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), TGL_BUFFER_OFFSET(sizeof(float) * 6));

	glGenBuffers(1, &m_sponza.instance_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_sponza.instance_VBO);
	glBufferData(GL_ARRAY_BUFFER, m_sponzaData.modelMatrices.size() * sizeof(glm::mat4), m_sponzaData.modelMatrices.data(), GL_STATIC_DRAW);

	// Set appropriate attrib pointers for instance buffer
	for (GLint i = 0; i < 4; i++)
	{
		glEnableVertexAttribArray(3 + i);
		glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), TGL_BUFFER_OFFSET(sizeof(float) * i * 4));
		glVertexAttribDivisor(3 + i, 1);
	}

	// Unbind VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


}

