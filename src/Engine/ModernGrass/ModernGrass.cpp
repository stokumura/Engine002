#include "ModernGrass.h"

ModernGrass::ModernGrass(const Model &model) : model(model) {
    const Mesh &mesh = model.GetMesh(0);
    count = mesh.GetIndexCount();
    VAO = mesh.GetVAO();

    glCreateBuffers(1, &SSBO);
    glNamedBufferStorage(SSBO, sizeof(SingleLeafData) * GX * GY, nullptr, 0);

    glCreateBuffers(1, &IndirectBuf);
    DrawElementsIndirectCommand cmd{ count, 0, 0, 0, 0 } ;
    glNamedBufferStorage(IndirectBuf, sizeof(DrawElementsIndirectCommand), &cmd, GL_DYNAMIC_STORAGE_BIT); 

}

void ModernGrass::Draw(const ShaderProgram &shaderProgram, const ShaderProgram &computeShaderProgram) const {

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
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IndirectBuf);
    glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr);
}
