#include "BillboardGrass.h"

BillboardGrass::BillboardGrass(const Texture &texture) : texture(texture), billboard(Vertex3DLit, 1, 1, 1, 1.0) {
    mesh = billboard.GetMesh();
    count = billboard.GetIndexCount();
    VAO = mesh.GetVAO();

    glCreateBuffers(1, &SSBO);
    glNamedBufferStorage(SSBO, sizeof(glm::vec4) * GX * GY, nullptr, 0);

    glCreateBuffers(1, &IndirectBuf);
    DrawElementsIndirectCommand cmd{ count, 0, 0, 0, 0 } ;
    glNamedBufferStorage(IndirectBuf, sizeof(DrawElementsIndirectCommand), &cmd, GL_DYNAMIC_STORAGE_BIT); 

}

void BillboardGrass::Draw(const ShaderProgram &shaderProgram, const ShaderProgram &computeShaderProgram, const Sampler &sampler) const {

    DrawElementsIndirectCommand reset { count, 0, 0, 0, 0 } ;
    glNamedBufferSubData(IndirectBuf, 0, sizeof(reset), &reset);

    computeShaderProgram.Bind();
    computeShaderProgram.SetInt("uGridX", GX);
    computeShaderProgram.SetInt("uGridY", GY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, IndirectBuf);
    glDispatchCompute((GX + 7) / 8, (GY + 7) / 8, 1);

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT | GL_COMMAND_BARRIER_BIT);

    shaderProgram.Bind();
    sampler.Bind(texture.GetUnit());
    texture.Bind();
    shaderProgram.SetInt("uAlbedoMap", texture.GetUnit());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IndirectBuf);
    glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr);
    texture.Unbind();
}
