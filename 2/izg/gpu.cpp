/*!
 * @file
 * @brief This file contains implementation of gpu
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */

#include <student/gpu.hpp>

struct Triangle{
    OutVertex points[3];
};

void loadTriangle(Triangle &triangle,Program &ptr,VertexArray &vao, uint32_t tId){
    for(int i = tId*3; i < tId*3+3; i++){ // smyčka přes vrcholy trojúhelníku
        InVertex inVertex;
        if(vao.indexBuffer == nullptr){
            inVertex.gl_VertexID = i;
        }
        else{
            if(vao.indexType == IndexType::UINT32)
                inVertex.gl_VertexID = ((uint32_t*)vao.indexBuffer)[i];
            else if(vao.indexType == IndexType::UINT16)
                inVertex.gl_VertexID = ((uint16_t*)vao.indexBuffer)[i];
            else
                inVertex.gl_VertexID = ((uint8_t*)vao.indexBuffer)[i];
        }

        VertexAttrib *att = vao.vertexAttrib;
        for(int j = 0; j < maxAttributes; j++){
            const void *adress = (char *)att[j].bufferData + att[j].offset + att[j].stride*inVertex.gl_VertexID;
            switch (att[j].type){
                case AttributeType::EMPTY:
                    continue;
                case AttributeType::FLOAT:
                    inVertex.attributes[j].v1 = *(float*)(adress);
                    break;
                case AttributeType::VEC2:
                    inVertex.attributes[j].v2 = *(glm::vec2*)(adress);
                    break;
                case AttributeType::VEC3:
                    inVertex.attributes[j].v3 = *(glm::vec3*)(adress);
                    break;
                case AttributeType::VEC4:
                    inVertex.attributes[j].v4 = *(glm::vec4*)(adress);
            }
        }   

        (*ptr.vertexShader)(triangle.points[i-tId*3],inVertex,ptr.uniforms);
    }
}

void perspectiveDivision(Triangle &triangle){
    for(int i = 0; i < 3; i++){
        triangle.points[i].gl_Position[0] = triangle.points[i].gl_Position[0] / triangle.points[i].gl_Position[3];
        triangle.points[i].gl_Position[1] = triangle.points[i].gl_Position[1] / triangle.points[i].gl_Position[3];
        triangle.points[i].gl_Position[2] = triangle.points[i].gl_Position[2] / triangle.points[i].gl_Position[3];
    }
}


void viewportTransformation(Triangle &triangle,uint32_t width, uint32_t height){
    for(int i = 0; i < 3; i++){
        triangle.points[i].gl_Position[0] = (triangle.points[i].gl_Position[0]*0.5 + 0.5) * width;
        triangle.points[i].gl_Position[1] = (triangle.points[i].gl_Position[1]*0.5 + 0.5) * height;
    }
}

#define Point(x) triangle.points[(x)].gl_Position

void barycentric(glm::vec4 p, Triangle &triangle, glm::vec3 &barCoor, glm::vec3 &barCoorCor){
    glm::vec4 v0 = Point(1) - Point(0), v1 = Point(2) - Point(0), v2 = p - Point(0);
    float den = v0.x * v1.y - v1.x * v0.y;
    barCoor.y = (v2.x * v1.y - v1.x * v2.y) / den;
    barCoor.z = (v0.x * v2.y - v2.x * v0.y) / den;
    barCoor.x = 1.0f - barCoor.y - barCoor.z;

    float s = barCoor.x/Point(0).w + barCoor.y/Point(1).w + barCoor.z/Point(2).w; 
    barCoorCor.x = barCoor.x / (Point(0).w*s);
    barCoorCor.y = barCoor.y / (Point(1).w*s);
    barCoorCor.z = barCoor.z / (Point(2).w*s);
}


void perFragmentOperations(Frame &frame, InFragment &inFragment, Program const &prg){
    OutFragment outFragment;

    prg.fragmentShader(outFragment,inFragment,prg.uniforms);
    int x = inFragment.gl_FragCoord.x;
    int y = inFragment.gl_FragCoord.y;
    float z = inFragment.gl_FragCoord.z;

    float a = outFragment.gl_FragColor[3]; 

    if(z < frame.depth[y*frame.width+x]){
        glm::vec4 newColor = glm::vec4(
                frame.color[(y*frame.width+x)*4]/255.f,
                frame.color[(y*frame.width+x)*4+1]/255.f,
                frame.color[(y*frame.width+x)*4+2]/255.f,
                0.f
                );

        newColor = glm::clamp(newColor*(1-a) + outFragment.gl_FragColor*a,0.f,1.f)*255.f;

        frame.color[(y*frame.width+x)*4]   = newColor[0];
        frame.color[(y*frame.width+x)*4+1] = newColor[1];
        frame.color[(y*frame.width+x)*4+2] = newColor[2];

        if(a > 0.5f)
            frame.depth[y*frame.width+x] = z;
    }

}


void drawLine(float x1, float y1, float x2, float y2, int lines[1000][2], int start, int end, Frame &frame)
{
    bool steep = abs(y2 - y1) > abs(x2 - x1);
    
    if (steep) { 
        int temp;
        temp = x1;
        x1 = y1;
        y1 = temp;
        temp = x2;
        x2 = y2;
        y2 = temp;
    }
    
    if (x1 > x2) {
        int temp;
        temp = x1;
        x1 = x2;
        x2 = temp;
        temp = y1;
        y1 = y2;
        y2 = temp;

    }
    
    const int dx = x2 - x1, dy = abs(y2 - y1);
    const int P1 = 2 * dy, P2 = P1 - 2 * dx;
    float y = y1;
    int ystep = 1;
    if (y1 > y2) ystep = -1;
    int P = 2*dy - dx;

    for (float x = x1; x <= x2; x++) {
        if (steep) { 
            if(x >= start && x <= end){
                lines[int(x)-start][0] = lines[int(x)-start][0] < int(y) ? lines[int(x)-start][0] : int(y);
                lines[int(x)-start][1] = lines[int(x)-start][1] > int(y) ? lines[int(x)-start][1] : int(y);
            }
        } else {
            if(y >= start && y <= end){
                lines[int(y)-start][0] = lines[int(y)-start][0] < int(x) ? lines[int(y)-start][0] : int(x);
                lines[int(y)-start][1] = lines[int(y)-start][1] > int(x) ? lines[int(y)-start][1] : int(x);
            }
        }

        if (P >= 0) {
            P += P2;
            y += ystep;
        } else { 
            P += P1;
        }
    }
}



void rasterize(Frame &frame,Triangle &triangle,Program const &prg, int lines[][2]){
    // frame okolo trojuholnika
    glm::vec4 min = glm::min(Point(0), Point(1));
    min = glm::clamp(glm::min(min, Point(2)), glm::vec4(0,0,0,0), glm::vec4(frame.width, frame.height,0,0));
    glm::vec4 max = glm::max(Point(0), Point(1));
    max = glm::clamp(glm::max(max, Point(2)), glm::vec4(0,0,0,0), glm::vec4(frame.width, frame.height,0,0));

    for(int i = 0; i < 1000; i++){
        lines[i][0] = max.x;
        lines[i][1] = min.x;
    }

    int start = int(min.y);
    drawLine(Point(0)[0]-0.5f, Point(0)[1]-0.5f, Point(1)[0]-0.5f, Point(1)[1]-0.5f, lines,start,int(max.y),frame);
    drawLine(Point(1)[0]-0.5f, Point(1)[1]-0.5f, Point(2)[0]-0.5f, Point(2)[1]-0.5f, lines,start,int(max.y),frame);
    drawLine(Point(2)[0]-0.5f, Point(2)[1]-0.5f, Point(0)[0]-0.5f, Point(0)[1]-0.5f, lines,start,int(max.y),frame);
 
    for(int y = min.y-start; y <= max.y-start; y++){
        for(int x = lines[y][0]; x <= lines[y][1]; x++){
            if(x < 0) continue;
            if(x >= frame.width) continue;

            InFragment inFragment;

            glm::vec4 point = glm::vec4(x+0.5f, y+start+0.5f,0.f,0.f);
            glm::vec3 barCoor, barCoorCor;
            barycentric(point, triangle, barCoor, barCoorCor);

            float z = barCoor.x*Point(0).z + barCoor.y*Point(1).z + barCoor.z*Point(2).z;   

            inFragment.gl_FragCoord = glm::vec4(x+0.5f, y+start+0.5f, z, 0.f);
            for(int i = 0; i < maxAttributes; i++){
                if(prg.vs2fs[i] == AttributeType::EMPTY)
                    continue;
                inFragment.attributes[i].v4 = barCoorCor.x*triangle.points[0].attributes[i].v4 +
                                            + barCoorCor.y*triangle.points[1].attributes[i].v4 +
                                            + barCoorCor.z*triangle.points[2].attributes[i].v4;
            }

            perFragmentOperations(frame,inFragment,prg);

        }

    }  

}

void clipping(Triangle &triangle, std::vector<Triangle> &result){
    std::vector<OutVertex> points;

    OutVertex previousVertex = triangle.points[2];
    float previousComponent = -previousVertex.gl_Position.z;
    bool previousInside = previousComponent <= previousVertex.gl_Position.w;

    for(int i = 0; i < 3; i++){
        OutVertex currentVertex = triangle.points[i];
        float currentComponent = -currentVertex.gl_Position.z;
        bool currentInside = currentComponent <= currentVertex.gl_Position.w;

        if(currentInside ^ previousInside){
           OutVertex nw = currentVertex;
            float lerpAmt = (previousVertex.gl_Position.w - previousComponent) /
                            ((previousVertex.gl_Position.w - previousComponent) - 
                             (currentVertex.gl_Position.w - currentComponent));
            nw.gl_Position = (previousVertex.gl_Position-currentVertex.gl_Position)*(1-lerpAmt)+currentVertex.gl_Position;

            glm::vec4 point = glm::vec4(nw.gl_Position.x, nw.gl_Position.y,0.f,0.f);
            glm::vec3 barCoor, barCoorCor;
            points.push_back(nw);
            
        }

        if(currentInside){
            points.push_back(currentVertex);
        }

        previousVertex = currentVertex;
        previousComponent = currentComponent;
        previousInside = currentInside; 
    }
    
    for(int i = 1; i+1 < points.size(); i++){
        Triangle tr;
        triangle.points[0] = points[0];
        triangle.points[1] = points[i];
        triangle.points[2] = points[i+1];
        result.push_back(triangle);
    }
}

//! [drawTrianglesImpl]
void drawTrianglesImpl(GPUContext &ctx,uint32_t nofVertices){
  for(int t = 0; t < nofVertices/3; t++){
        Triangle triangle_orig;
        loadTriangle(triangle_orig,ctx.prg,ctx.vao,t);

         
        int lines[3000][2];

        std::vector<Triangle> triangles;
        clipping(triangle_orig, triangles);
        for (auto &triangle : triangles){
            perspectiveDivision(triangle);
            viewportTransformation(triangle,ctx.frame.width,ctx.frame.height);
            rasterize(ctx.frame,triangle,ctx.prg,lines);
        }
        
    }
  /// \todo Tato funkce vykreslí trojúhelníky podle daného nastavení.<br>
  /// ctx obsahuje aktuální stav grafické karty.
  /// Parametr "nofVertices" obsahuje počet vrcholů, který by se měl vykreslit (3 pro jeden trojúhelník).<br>
  /// Bližší informace jsou uvedeny na hlavní stránce dokumentace.
}
//! [drawTrianglesImpl]

/**
 * @brief This function reads color from texture.
 *
 * @param texture texture
 * @param uv uv coordinates
 *
 * @return color 4 floats
 */
glm::vec4 read_texture(Texture const&texture,glm::vec2 uv){
  if(!texture.data)return glm::vec4(0.f);
  auto uv1 = glm::fract(uv);
  auto uv2 = uv1*glm::vec2(texture.width-1,texture.height-1)+0.5f;
  auto pix = glm::uvec2(uv2);
  //auto t   = glm::fract(uv2);
  glm::vec4 color = glm::vec4(0.f,0.f,0.f,1.f);
  for(uint32_t c=0;c<texture.channels;++c)
    color[c] = texture.data[(pix.y*texture.width+pix.x)*texture.channels+c]/255.f;
  return color;
}

/**
 * @brief This function clears framebuffer.
 *
 * @param ctx GPUContext
 * @param r red channel
 * @param g green channel
 * @param b blue channel
 * @param a alpha channel
 */
void clear(GPUContext&ctx,float r,float g,float b,float a){
  auto&frame = ctx.frame;
  auto const nofPixels = frame.width * frame.height;
  for(size_t i=0;i<nofPixels;++i){
    frame.depth[i] = 10e10f;
    frame.color[i*4+0] = static_cast<uint8_t>(glm::min(r*255.f,255.f));
    frame.color[i*4+1] = static_cast<uint8_t>(glm::min(g*255.f,255.f));
    frame.color[i*4+2] = static_cast<uint8_t>(glm::min(b*255.f,255.f));
    frame.color[i*4+3] = static_cast<uint8_t>(glm::min(a*255.f,255.f));
  }
}

