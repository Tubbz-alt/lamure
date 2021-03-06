// Copyright (c) 2014-2018 Bauhaus-Universitaet Weimar
// This Software is distributed under the Modified BSD License, see license.txt.
//
// Virtual Reality and Visualization Research Group 
// Faculty of Media, Bauhaus-Universitaet Weimar
// http://www.uni-weimar.de/medien/vr

#include "sampler.h"
#include <vcg/space/triangle3.h>

#include <unordered_map>


static unsigned OUTPUT_FREQ = 14000;

void sampler::
compute_normals()
{
#ifndef USE_WEDGE_NORMALS
    //PerVertexclear(m);
    for (auto& f : m.face)
        if (!f.IsD() && f.IsR()) {
          //typename FaceType::NormalType t = (*f).Normal();

          auto t = vcg::TriangleNormal(f);

            for (int j = 0; j < 3; ++j) {
              if (!f.V(j)->IsD() && f.V(j)->IsRW())
              {
                f.V(j)->N() += vcg::Point3f(t.X(), t.Y(), t.Z());
              }
            }
        }
#endif
}

bool sampler::
load(const std::string& filename, std::string const& attribute_texture_suffix)
{
    typedef vcg::tri::io::Importer<MyMesh> Importer;

    vcg::tri::RequirePerFaceWedgeTexCoord(m);

    int mask = -1;

    std::cout << "loading... " << std::flush;
    int result = Importer::Open(m, filename.c_str(), mask);

    if (Importer::ErrorCritical(result)) {
        std::cerr << std::endl << "Unable to load file: \"" << filename 
                  << "\". " << Importer::ErrorMsg(result) << std::endl;
        return false;
    }
    std::cout << "DONE" << std::endl;

    if (result != 0)
        std::cout << "Warning: " << Importer::ErrorMsg(result) << std::endl;
    
    //if (!(mask & vcg::tri::io::Mask::IOM_VERTTEXCOORD))
    //    std::cerr << "Vertex texture coords are not available" << std::endl;
    
    if (!(mask & vcg::tri::io::Mask::IOM_WEDGTEXCOORD))
        std::cerr << "Wedge texture coords are not available. Sampling will fail!" << std::endl;

#ifdef USE_WEDGE_NORMALS
    if (!(mask & vcg::tri::io::Mask::IOM_WEDGNORMAL))
        std::cerr << "Wedge normals are not available. Sampling will fail!" << std::endl;
#else
    if (!(mask & vcg::tri::io::Mask::IOM_VERTNORMAL)) {
        std::cerr << "Vertex normals are not available. Recompute... " << std::flush;
        vcg::tri::UpdateNormal<MyMesh>::PerVertexClear(m);
        compute_normals();
        vcg::tri::UpdateNormal<MyMesh>::NormalizePerVertex(m);
        std::cout << "DONE" << std::endl;
    }
#endif

    std::cout << "Topology update... " << std::flush;
    vcg::tri::UpdateTopology<MyMesh>::VertexFace(m);
    std::cout << "DONE" << std::endl;

    //vcg::tri::RequirePerVertexNormal(m);
    std::cout << "Mesh: " << m.VN() << " vertices, " << m.FN() << " faces." << std::endl;

    textures = std::vector<texture>(m.textures.size());
    attribute_textures = std::vector<texture>(m.textures.size());

    #pragma omp parallel for
    for (size_t i = 0; i < m.textures.size(); ++i) {
#if 1
        //for relative path in mtl:
        unsigned slashPos = std::string(filename).find_last_of("/\\");
        std::string textureFileName = std::string(filename).substr(0, slashPos+1) + m.textures[i];
#else
        //for absolute path in mtl:
        std::string textureFileName = m.textures[i];
#endif
        textures[i].load(textureFileName);

        unsigned file_extension_pos = std::string(textureFileName).find_last_of(".");
        std::string const pre_suffix_filename = std::string(textureFileName).substr(0, file_extension_pos);
        std::string const file_extension = std::string(textureFileName).substr(file_extension_pos);
        //attribute_texture_suffix

        if("" != attribute_texture_suffix) {
            std::string const attribute_texture_name =  pre_suffix_filename + attribute_texture_suffix + file_extension;        
            attribute_textures[i].load(attribute_texture_name);
        }
    }

    return true;
}

bool sampler::
SampleMesh(const std::string& outputFilename, bool flip_x, bool flip_y)
{
    std::ofstream out(outputFilename, std::ios::out);
    if (out.fail()) {
        std::cerr << "Unable to create file: \"" << outputFilename << "\". " 
                  << strerror(errno) << std::endl;
        out.close();
        return false;
    }   

    unsigned processedFaces = 0,
             processedPoints = 0,
             discardedFaces = 0;

    std::cout.setf(ios::fixed, ios::floatfield);
    std::cout.precision(1);

    std::cout << "Sampling mesh... " << std::endl;

    surfel_vector points;

    #pragma omp parallel for private(points) schedule(dynamic, 35)
    for (size_t i = 0; i < m.face.size(); ++i) {
        bool discard = false;
        if (sample_face(i, flip_x, flip_y, points)) {
            
        } else
            discard = true;

        #pragma omp critical(save)
        {
            char buffer[256];
            for (auto& p: points) {
                if (p.d <= 0.0) continue;
                int sz = sprintf(buffer, "%f %f %f %f %f %f %u %u %u %f %u\n", p.x,p.y,p.z,p.nx,p.ny,p.nz,p.r,p.g,p.b,p.d,p.a); // a = additional attribute, later stored in alpha
                out.write(buffer, sz);
                ++processedPoints;
            }
            ++processedFaces;
            if (discard)
                ++discardedFaces;

            if (processedFaces % OUTPUT_FREQ == 0 || processedFaces == m.face.size())
                std::cout << float(processedFaces*100)/m.face.size()
                          <<"%. Faces processed: " << processedFaces<< " / " 
                          << m.face.size() << std::endl;
        }

    }
    out.close();

    std::cout << "DONE" << std::endl;
    std::cout <<"Faces processed: " << processedFaces
              << ". Discarded: " << discardedFaces << std::endl
              << "Points generated: " << processedPoints << std::endl;
    return true;
}

bool sampler::
sample_face(int faceId, 
  bool flip_x, bool flip_y,
  surfel_vector& out) 
{
    return sample_face(&m.face[faceId], flip_x, flip_y, out);
}

namespace {
    // Helper method to emulate GLSL
    float fract(float value){
    return (float)fmod(value, 1.0f);
    }
}

bool sampler::
sample_face(face_pointer facePtr, 
  bool flip_x, bool flip_y,
  surfel_vector& out)
{
    out.clear();

    auto& face = *facePtr;

    if (face.IsD() || !face.IsR()) {
        return false;
    }

    short texId = face.WT(0).N();
    if (texId < 0 || size_t(texId) >= textures.size()) {
        return false;
    }

    texture& tex = textures[texId];
    if (!tex.is_loaded()) {
        return false;
    }

    bool sample_alpha_attribute = true;
    texture& attribute_tex = attribute_textures[texId];
    if (!attribute_tex.is_loaded()) {
        sample_alpha_attribute = false;
    }

    // calculate 2D vertex positions [0, tex_width-1][0, tex_height-1] in ints
    
    float tax, tbx, tcx;
    float tay, tby, tcy;

    if (flip_x) {
      tax = (1.0 - fract(face.WT(0).U())) * (tex.width() - 1) + 0.5f;
      tbx = (1.0 - fract(face.WT(1).U())) * (tex.width() - 1) + 0.5f;
      tcx = (1.0 - fract(face.WT(2).U())) * (tex.width() - 1) + 0.5f;
    }
    else {
      tax = fract(face.WT(0).U()) * (tex.width() - 1) + 0.5f;
      tbx = fract(face.WT(1).U()) * (tex.width() - 1) + 0.5f;
      tcx = fract(face.WT(2).U()) * (tex.width() - 1) + 0.5f;
    }

    if (flip_y) {
      tay = (1.0 - fract(face.WT(0).V())) * (tex.height() - 1) + 0.5f;
      tby = (1.0 - fract(face.WT(1).V())) * (tex.height() - 1) + 0.5f;
      tcy = (1.0 - fract(face.WT(2).V())) * (tex.height() - 1) + 0.5f;
    }
    else {
      tay = fract(face.WT(0).V()) * (tex.height() - 1) + 0.5f;
      tby = fract(face.WT(1).V()) * (tex.height() - 1) + 0.5f;
      tcy = fract(face.WT(2).V()) * (tex.height() - 1) + 0.5f;
    }

    vcg::Triangle2<double> T(vcg::Point2d(tax,tay), vcg::Point2d(tbx, tby), vcg::Point2d(tcx, tcy));

    int minXTri = std::min(tax, std::min(tbx, tcx));
    int maxXTri = std::max(tax, std::max(tbx, tcx));
    int minYTri = std::min(tay, std::min(tby, tcy));
    int maxYTri = std::max(tay, std::max(tby, tcy));

    const vcg::Point3d A = face.V(0)->P();
    const vcg::Point3d B = face.V(1)->P();
    const vcg::Point3d C = face.V(2)->P();

    vcg::Point3d CA = C-A;
    vcg::Point3d BA = B-A;

    double areaABC = area(A,B,C); //whole triangle


// new

    vcg::Box3d trBox;
    trBox.Add(A);
    trBox.Add(B);
    trBox.Add(C);
    double maxRadius = trBox.Diag() / 2.0;
   
    // compute diameter
    auto MapPoint = [&](int x, int y) {
        double u, v;
        is_point_in_triangle(T, vcg::Point2d(x, y), u, v);
        return A + CA*u + BA*v;
    };
    vcg::Point3d mapped[3] = {MapPoint(0, 0), MapPoint(1, 0), MapPoint(0, 1)};
    double rad = std::max(vcg::Distance(mapped[0], mapped[1]), vcg::Distance(mapped[0], mapped[2]));
    rad = std::min(rad, maxRadius);


    float scaling_color_tex_to_attribute_tex_x = 1.0;
    float scaling_color_tex_to_attribute_tex_y = 1.0;

    if(sample_alpha_attribute) {
        scaling_color_tex_to_attribute_tex_x = attribute_tex.width() / float(tex.width()); 
        scaling_color_tex_to_attribute_tex_y = attribute_tex.height() / float(tex.height()); 
    }

    for (int x = minXTri; x <= maxXTri; ++x) {
        for (int y = minYTri; y <= maxYTri; ++y) {

            double u, v; //barycentric parameter to reconstruct 3D pos

            if (!is_point_in_triangle(T, vcg::Point2d(x, y), u, v))
                continue;

            vcg::Point3d P = A + CA*u + BA*v;

            //barycentric interpolation of normal values	
            //sub triangles defined trough two vertices of whole triangle and point in triangle
            double areaABP = area(A,B,P); 
            double areaACP = area(A,C,P); 
            double areaBCP = area(B,C,P);
            
#ifdef USE_WEDGE_NORMALS
            vcg::Point3f norm[3] = {face.WN(0), face.WN(1), face.WN(2)};
#else
            vcg::Point3f norm[3] = {face.V(0)->N(), face.V(1)->N(), face.V(2)->N()};
#endif

            vcg::Point3f NP = (norm[0]*areaBCP + norm[1]*areaACP + norm[2]*areaABP) / areaABC;
            NP.Normalize();

            //fetch color from texture
            texture::pixel texel = tex.get_pixel(x, y);

            unsigned char alpha_attribute = 0;
            if(sample_alpha_attribute) {
                                                          //needs to be scaled because attribute and color texture could differ in size
                alpha_attribute = attribute_tex.get_pixel(scaling_color_tex_to_attribute_tex_x*x, 
                                                          scaling_color_tex_to_attribute_tex_y*y).r;

//                if(alpha_attribute > 0) {
            //        std::cout << "Sampled attribute: " << int(alpha_attribute) << std::endl;
//                }
            }


            if (1/*texel.a == (unsigned char)255*/) {
              out.push_back({P.X(), P.Y(), P.Z(), NP.X(), NP.Y(), NP.Z(), texel.r, texel.g, texel.b, alpha_attribute, rad});
              //out.push_back({P.X(), P.Y(), P.Z(), NP.X(), NP.Y(), NP.Z(), alpha_attribute, alpha_attribute, alpha_attribute, alpha_attribute, rad});
            }
        }
    }


    return true;
}

