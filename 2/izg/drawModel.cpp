/*!
 * @file
 * @brief This file contains functions for model rendering
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */
#include <student/drawModel.hpp>
#include <student/gpu.hpp>

void drawNode(GPUContext&ctx,Node const&node,Model const&model,glm::mat4 const&prubeznaMatice){
    glm::mat4 nodeMatrix;
    nodeMatrix = prubeznaMatice * node.modelMatrix;
    

  if(node.mesh>=0){
    auto mesh = model.meshes[node.mesh];

    //ctx.voa.vertexAttrib = mesh.position;

        ctx.prg.uniforms.uniform[1].m4 = nodeMatrix;
    ctx.prg.uniforms.uniform[2].m4 = glm::transpose(glm::inverse(nodeMatrix));
    ctx.prg.uniforms.uniform[5].v4 = mesh.diffuseColor;

    if(mesh.diffuseTexture != -1){
        ctx.prg.uniforms.textures[0] = model.textures[mesh.diffuseTexture];
        ctx.prg.uniforms.uniform[6].v1 = 1.f;
    }
    else{
        ctx.prg.uniforms.textures[0].data = nullptr;
        ctx.prg.uniforms.textures[0].width = 0;
        ctx.prg.uniforms.textures[0].height = 0;
        ctx.prg.uniforms.textures[0].channels = 3;
        ctx.prg.uniforms.uniform[6].v1 = 0.f;
    }    

    ctx.vao.vertexAttrib[0] = mesh.position;
    ctx.vao.indexBuffer = mesh.indices;
    //if(mesh.diffuseTexture>=0)
    //  ctx.prg.uniforms.textures[0] = model.textures[mesh.diffuseTexture];
    //...
    //drawTriangles(ctx,mesh.nofIndices);

    //position
    ctx.vao.vertexAttrib[0] = mesh.position;
    ctx.vao.vertexAttrib[1] = mesh.normal;
    ctx.vao.vertexAttrib[2] = mesh.texCoord;

    //normal
    //ctx.vao.vertexAttrib[1].bufferData = bunnyVertices      ;
    //ctx.vao.vertexAttrib[1].type       = AttributeType::VEC3;
    //ctx.vao.vertexAttrib[1].stride     = sizeof(BunnyVertex);
    //ctx.vao.vertexAttrib[1].offset     = sizeof(glm::vec3)  ;

    ctx.vao.indexBuffer = mesh.indices  ;
    ctx.vao.indexType   = mesh.indexType;

    ctx.prg.vertexShader   = drawModel_vertexShader;
    ctx.prg.fragmentShader = drawModel_fragmentShader;
    //ctx.prg.vs2fs[0]       = mesh.position.type;
    //ctx.prg.vs2fs[1]       = mesh.normal.type;
    //ctx.prg.vs2fs[2]       = mesh.texCoord.type;
    ctx.prg.vs2fs[0]       = AttributeType::VEC3;
    ctx.prg.vs2fs[1]       = AttributeType::VEC3;
    ctx.prg.vs2fs[2]       = AttributeType::VEC2;



    //ctx.frame = frame;
    //clear(ctx,.5f,.5f,.5f,1.f);
    //ctx.prg.uniforms.uniform[0].m4 = view  ;
    //ctx.prg.uniforms.uniform[1].m4 = proj  ;
    //ctx.prg.uniforms.uniform[2].v3 = light ;
    //ctx.prg.uniforms.uniform[3].v3 = camera;
    drawTriangles(ctx,mesh.nofIndices);

  }

  for(size_t i=0;i<node.children.size();++i)
    drawNode(ctx,node.children[i],model,nodeMatrix);
}

/**
 * @brief This function renders a model
 *
 * @param ctx GPUContext
 * @param model model structure
 * @param proj projection matrix
 * @param view view matrix
 * @param light light position
 * @param camera camera position (unused)
 */
//! [drawModel]
void drawModel(GPUContext&ctx,Model const&model,glm::mat4 const&proj,glm::mat4 const&view,glm::vec3 const&light,glm::vec3 const&camera){
  (void)ctx;
  (void)model;
  (void)proj;
  (void)view;
  (void)light;
  (void)camera;

  ctx.prg.uniforms.uniform[0].m4 = proj*view; 
  ctx.prg.uniforms.uniform[3].v3 = light;

  glm::mat4 jednotkovaMatrice = glm::mat4(1.f);
  for(size_t i=0;i<model.roots.size();++i)
      drawNode(ctx,model.roots[i],model,jednotkovaMatrice);

    /// \todo Tato funkce vykreslí model.<br>
  /// Vaším úkolem je správně projít model a vykreslit ho pomocí funkce drawTriangles (nevolejte drawTrianglesImpl, je to z důvodu testování).
  /// Bližší informace jsou uvedeny na hlavní stránce dokumentace.
}
//! [drawModel]

/**
 * @brief This function represents vertex shader of texture rendering method.
 *
 * @param outVertex output vertex
 * @param inVertex input vertex
 * @param uniforms uniform variables
 */
//! [drawModel_vs]
void drawModel_vertexShader(OutVertex&outVertex,InVertex const&inVertex,Uniforms const&uniforms){
  (void)outVertex;
  (void)inVertex;
  (void)uniforms;
  auto const pos = glm::vec4(inVertex.attributes[0].v3,1.f);
  auto const&nor = glm::vec4(inVertex.attributes[1].v3,1.f);
  auto const&text = inVertex.attributes[2].v2;
  auto const&viewMatrix  = uniforms.uniform[0].m4;
  auto const&modelMatrix = uniforms.uniform[1].m4;
  auto const&modelMatrixInv = uniforms.uniform[2].m4;
 
  auto mvp = viewMatrix * modelMatrix;
 
  outVertex.gl_Position = mvp * pos;
  outVertex.attributes[0].v3 = modelMatrix * pos;
  outVertex.attributes[1].v3 = modelMatrixInv * nor;
  outVertex.attributes[2].v2 = text;
  

  /// \todo Tato funkce reprezentujte vertex shader.<br>
  /// Vaším úkolem je správně trasnformovat vrcholy modelu.
  /// Bližší informace jsou uvedeny na hlavní stránce dokumentace.
}
//! [drawModel_vs]

/**
 * @brief This functionrepresents fragment shader of texture rendering method.
 *
 * @param outFragment output fragment
 * @param inFragment input fragment
 * @param uniforms uniform variables
 */
//! [drawModel_fs]
void drawModel_fragmentShader(OutFragment&outFragment,InFragment const&inFragment,Uniforms const&uniforms){
  (void)outFragment;
  (void)inFragment;
  (void)uniforms;

  auto const& light    = uniforms.uniform[3].v3;
  auto const& difColor = uniforms.uniform[5].v4;
  auto const& texture  = uniforms.uniform[6].v1;

  auto const& pos = inFragment.attributes[0].v3;
  auto const& nor = inFragment.attributes[1].v3;
  auto const& tex = inFragment.attributes[2].v2;

  auto nor_normalized = glm::normalize(nor);
  auto l = glm::normalize(light-pos);

  float diffuseFactor = glm::clamp(glm::dot(l, nor_normalized),0.f,1.f);

  
  glm::vec4 dC;
  if(texture > 0.f){
      dC = read_texture(uniforms.textures[0],tex);
  }
  else{
      dC = difColor;
  }

  glm::vec4 aL = dC * 0.2f;
  glm::vec4 dL = dC * diffuseFactor;

  glm::vec4 color = aL + dL;
  color.w = dC.w;

  outFragment.gl_FragColor = color;
  /// \todo Tato funkce reprezentujte fragment shader.<br>
  /// Vaším úkolem je správně obarvit fragmenty a osvětlit je pomocí lambertova osvětlovacího modelu.
  /// Bližší informace jsou uvedeny na hlavní stránce dokumentace.
}
//! [drawModel_fs]

