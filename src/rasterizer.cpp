#include "rasterizer.h"

using namespace std;

namespace CGL {

  RasterizerImp::RasterizerImp(PixelSampleMethod psm, LevelSampleMethod lsm,
    size_t width, size_t height,
    unsigned int sample_rate) {
    this->psm = psm;
    this->lsm = lsm;
    this->width = width;
    this->height = height;
    this->sample_rate = sample_rate;

    sample_buffer.resize(width * height * sample_rate, Color::White);
  }

  // Used by rasterize_point and rasterize_line
  void RasterizerImp::fill_pixel(size_t x, size_t y, Color c) {
    // TODO: Task 2: You might need to this function to fix points and lines (such as the black rectangle border in test4.svg)
    // NOTE: You are not required to implement proper supersampling for points and lines
    // It is sufficient to use the same color for all supersamples of a pixel for points and lines (not triangles)
    int super_width = (int) sqrt((double) sample_rate);
    int off_x = super_width * floor(x);
    int off_y = super_width * floor(y);
    
    // Bound Check
    if (off_x < 0 || (off_x + super_width - 1) >= super_width * width) {
      cout << "Fill Pixel: x out of bounds.";
      return;
    }
    if (off_y < 0 || (off_y + super_width - 1) >= super_width * height) {
      cout << "Fill Pixel: y out of bounds.";
      return;
    }
    
    for (int i = 0; i < super_width; i++) {
      for (int j = 0; j < super_width; j++) {
        sample_buffer[(j + off_y) * width * super_width + (i + off_x)] = c;
      }
    }
  }

  // Rasterize a point: simple example to help you start familiarizing
  // yourself with the starter code.
  //
  void RasterizerImp::rasterize_point(float x, float y, Color color) {
    // fill in the nearest pixel
    int sx = (int)floor(x);
    int sy = (int)floor(y);

    // check bounds
    if (sx < 0 || sx >= width) return;
    if (sy < 0 || sy >= height) return;

    fill_pixel(sx, sy, color);
    return;
  }

  // Rasterize a line.
  void RasterizerImp::rasterize_line(float x0, float y0,
    float x1, float y1,
    Color color) {
    if (x0 > x1) {
      swap(x0, x1); swap(y0, y1);
    }

    float pt[] = { x0,y0 };
    float m = (y1 - y0) / (x1 - x0);
    float dpt[] = { 1,m };
    int steep = abs(m) > 1;
    if (steep) {
      dpt[0] = x1 == x0 ? 0 : 1 / abs(m);
      dpt[1] = x1 == x0 ? (y1 - y0) / abs(y1 - y0) : m / abs(m);
    }

    while (floor(pt[0]) <= floor(x1) && abs(pt[1] - y0) <= abs(y1 - y0)) {
      rasterize_point(pt[0], pt[1], color);
      pt[0] += dpt[0]; pt[1] += dpt[1];
    }
  }

  // Rasterize a triangle.
  void RasterizerImp::rasterize_triangle(float x0, float y0,
    float x1, float y1,
    float x2, float y2,
    Color color) {
    
    // TODO: Task 1: Implement basic triangle rasterization here, no supersampling
    // TODO: Task 2: Update to implement super-sampled rasterization
    
    // Structure setup and scaling.
    int scale = sqrt(sample_rate);
    float xs [3] = {x0, x1, x2};
    float ys [3] = {y0, y1, y2};
    for (int i = 0; i < 3; i++) {
      xs[i] *= scale;
      ys[i] *= scale;
    }
    
    // Find bounding box of triange.
    float minX = *min_element(xs, xs+3);
    float minY = *min_element(ys, ys+3);
    float maxX = *max_element(xs, xs+3);
    float maxY = *max_element(ys, ys+3);
    
    // Direction Selection - Clockwise or Counterclockwise
    vector<vector<int>> counterclockwise = {{0,1},{1,2},{2,0}};
    vector<vector<int>> clockwise = {{0,2},{2,1},{1,0}};
    vector<vector<int>> tests = counterclockwise;
    float dx = xs[1] - xs[0];
    float dy = ys[1] - ys[0];
    float diff = -(xs[2] - xs[0]) * dy + (ys[2] - ys[0]) * dx;
    if (diff < 0) tests = clockwise;
    
    // Iterate over points in bounding box of triangle
    for (int i = floor(minX); i <= ceil(maxX); i++) {
      for (int j = floor(minY); j <= ceil(maxY); j++) {
        
        // Point-in-Triangle Test
        float pt [2] = {(float) i, (float)j}; pt[0] += 0.5; pt[1] += 0.5;
        bool pass = true;
        for (int test = 0; test < tests.size(); test++) {
          vector<int> currTest = tests[test];
          dx = xs[currTest[1]] - xs[currTest[0]];
          dy = ys[currTest[1]] - ys[currTest[0]];
          diff = -(pt[0] - xs[currTest[0]]) * dy + (pt[1] - ys[currTest[0]]) * dx;
          pass = pass && (diff >= 0);
        }
          
        // Check Bounds
        pass = pass && !(i < 0 || j < 0);
        pass = pass && !(i >= scale * width || j >= scale * height);
        
        // Render if pass.
        if (pass) sample_buffer[j * scale * width + i] = color;
        
      }
    }
  }


  void RasterizerImp::rasterize_interpolated_color_triangle(float x0, float y0, Color c0,
    float x1, float y1, Color c1,
    float x2, float y2, Color c2)
  {
    // TODO: Task 4: Rasterize the triangle, calculating barycentric coordinates and using them to interpolate vertex colors across the triangle
    // Hint: You can reuse code from rasterize_triangle
    
    // Structure setup and scaling.
    int scale = sqrt(sample_rate);
    float xs [3] = {x0, x1, x2};
    float ys [3] = {y0, y1, y2};
    for (int i = 0; i < 3; i++) {
      xs[i] *= scale;
      ys[i] *= scale;
    }
    
    // Find bounding box of triange.
    float minX = *min_element(xs, xs+3);
    float minY = *min_element(ys, ys+3);
    float maxX = *max_element(xs, xs+3);
    float maxY = *max_element(ys, ys+3);
    
    // Direction Selection - Clockwise or Counterclockwise
    vector<vector<int>> counterclockwise = {{0,1},{1,2},{2,0}};
    vector<vector<int>> clockwise = {{0,2},{2,1},{1,0}};
    vector<vector<int>> tests = counterclockwise;
    float dx = xs[1] - xs[0];
    float dy = ys[1] - ys[0];
    float diff = -(xs[2] - xs[0]) * dy + (ys[2] - ys[0]) * dx;
    if (diff < 0) tests = clockwise;
    
    // Iterate over points in bounding box of triangle
    for (int i = floor(minX); i <= ceil(maxX); i++) {
      for (int j = floor(minY); j <= ceil(maxY); j++) {
        
        // Point-in-triangle test
        float pt [2] = {(float) i, (float)j}; pt[0] += 0.5; pt[1] += 0.5;
        bool pass = true;
        for (int test = 0; test < tests.size(); test++) {
          vector<int> currTest = tests[test];
          dx = xs[currTest[1]] - xs[currTest[0]];
          dy = ys[currTest[1]] - ys[currTest[0]];
          diff = -(pt[0] - xs[currTest[0]]) * dy + (pt[1] - ys[currTest[0]]) * dx;
          pass = pass && (diff >= 0);
        }
        
        // Check Bounds
        pass = pass && !(i < 0 || j < 0);
        pass = pass && !(i >= scale * width || j >= scale * height);
        if (!pass) continue;
        
        // Compute first 2 Barycentric coordinates using Geometric interpretation.
        float coords [3];
        float triangle_area = abs(xs[0]*ys[1] + xs[1]*ys[2] + xs[2]*ys[0]
                                  - ys[0]*xs[1] - ys[1]*xs[2] -ys[2]*xs[0]) / 2;
        for (int vp = 1; vp < 3; vp++) {
          vector<int> vpair = counterclockwise[vp];
          float subarea = abs(xs[vpair[0]]*ys[vpair[1]] + xs[vpair[1]]*pt[1] + pt[0]*ys[vpair[0]]
                              - ys[vpair[0]]*xs[vpair[1]] - ys[vpair[1]]*pt[0] - pt[1]*xs[vpair[0]]) / 2;
          coords[3 - vpair[0] - vpair[1]] = subarea / triangle_area;
        }
        coords[2] = 1 - coords[0] - coords[1];
        
        // Interpolate Color
        Color color = (c0 * coords[0])+ (c1 * coords[1]) + (c2 * coords[2]);
        
        // Render.
        sample_buffer[j * scale * width + i] = color;
      }
    }
  }


  void RasterizerImp::rasterize_textured_triangle(float x0, float y0, float u0, float v0,
    float x1, float y1, float u1, float v1,
    float x2, float y2, float u2, float v2,
    Texture& tex)
  {
    // TODO: Task 5: Fill in the SampleParams struct and pass it to the tex.sample function.
    // TODO: Task 6: Set the correct barycentric differentials in the SampleParams struct.
    
    // Set up SampleParams
    SampleParams sp = SampleParams();
    sp.psm = this->psm;
    sp.lsm = this->lsm;
    
    // Structure setup and scaling.
    int scale = sqrt(sample_rate);
    float xs [3] = {x0, x1, x2};
    float ys [3] = {y0, y1, y2};
    float us [3] = {u0, u1, u2};
    float vs [3] = {v0, v1, v2};
    for (int i = 0; i < 3; i++) {
      xs[i] *= scale;
      ys[i] *= scale;
    }
    
    // Find bounding box of triange.
    float minX = *min_element(xs, xs+3);
    float minY = *min_element(ys, ys+3);
    float maxX = *max_element(xs, xs+3);
    float maxY = *max_element(ys, ys+3);
    
    // Direction Selection - Clockwise or Counterclockwise
    vector<vector<int>> counterclockwise = {{0,1},{1,2},{2,0}};
    vector<vector<int>> clockwise = {{0,2},{2,1},{1,0}};
    vector<vector<int>> tests = counterclockwise;
    float dx = xs[1] - xs[0];
    float dy = ys[1] - ys[0];
    float diff = -(xs[2] - xs[0]) * dy + (ys[2] - ys[0]) * dx;
    if (diff < 0) tests = clockwise;
    
    // Iterate over points in bounding box of triangle
    for (int i = floor(minX); i <= ceil(maxX); i++) {
      for (int j = floor(minY); j <= ceil(maxY); j++) {
        
        // Point-in-triangle test - (x,y) + neighbors.
        float pt [2] = {(float) i, (float)j}; pt[0] += 0.5; pt[1] += 0.5;
        bool pass = true; bool dx_pass = true; bool dy_pass = true;
        float diff_xy, diff_dx, diff_dy;
        for (int test = 0; test < tests.size(); test++) {
          vector<int> currTest = tests[test];
          dx = xs[currTest[1]] - xs[currTest[0]];
          dy = ys[currTest[1]] - ys[currTest[0]];
          diff_xy = -(pt[0] - xs[currTest[0]]) * dy + (pt[1] - ys[currTest[0]]) * dx;
          diff_dx = -(pt[0] + 1 - xs[currTest[0]]) * dy + (pt[1] - ys[currTest[0]]) * dx;
          diff_dy = -(pt[0] - xs[currTest[0]]) * dy + (pt[1] - ys[currTest[0]]) * dx;
          pass = pass && (diff_xy >= 0);
          dx_pass = dx_pass && (diff_dx >= 0);
          dy_pass = dy_pass && (diff_dy >= 0);
        }
        
        // Check Bounds
        pass = pass && !(i < 0 || j < 0);
        pass = pass && !(i >= scale * width || j >= scale * height);
        if (!pass) continue;
        
        // Compute first 2 Barycentric coordinates using Geometric interpretation.
        float coords [3], coords_dx [3], coords_dy [3];
        float triangle_area = abs(xs[0]*ys[1] + xs[1]*ys[2] + xs[2]*ys[0]
                                  - ys[0]*xs[1] - ys[1]*xs[2] -ys[2]*xs[0]) / 2;
        for (int vp = 1; vp < 3; vp++) {
          vector<int> vpair = counterclockwise[vp];
          float subarea = abs(xs[vpair[0]]*ys[vpair[1]] + xs[vpair[1]]*pt[1] + pt[0]*ys[vpair[0]]
                              - ys[vpair[0]]*xs[vpair[1]] - ys[vpair[1]]*pt[0] - pt[1]*xs[vpair[0]]) / 2;
          float subarea_dx = abs(xs[vpair[0]]*ys[vpair[1]] + xs[vpair[1]]*pt[1] +  (pt[0]+1)*ys[vpair[0]]
                              - ys[vpair[0]]*xs[vpair[1]] - ys[vpair[1]]*(pt[0]+1) - pt[1]*xs[vpair[0]]) / 2;
          float subarea_dy = abs(xs[vpair[0]]*ys[vpair[1]] + xs[vpair[1]]*(pt[1]+1) + pt[0]*ys[vpair[0]]
                              - ys[vpair[0]]*xs[vpair[1]] - ys[vpair[1]]*pt[0] - (pt[1]+1)*xs[vpair[0]]) / 2;
          coords[3 - vpair[0] - vpair[1]] = subarea / triangle_area;
          coords_dx[3 - vpair[0] - vpair[1]] = subarea_dx / triangle_area;
          coords_dy[3 - vpair[0] - vpair[1]] = subarea_dy / triangle_area;
        }
        coords[2] = 1 - coords[0] - coords[1];
        coords_dx[2] = 1 - coords_dx[0] - coords_dx[1];
        coords_dy[2] = 1 - coords_dy[0] - coords_dy[1];
        
        // Sample Color
        
        // Compute p_uv
        float u = (us[0] * coords[0])+(us[1] * coords[1])+(us[2] * coords[2]);
        float v = (vs[0] * coords[0])+(vs[1] * coords[1])+(vs[2] * coords[2]);
        // Clip to u v bounds. Ensures Correctness of Bilinear Filtering.
        if (u < 0) u = 0; if (u >= 1) u = 1; if(isnan(u)) return;
        if (v < 0) v = 0; if (v >= 1) v = 1; if(isnan(v)) return;
        Vector2D p_uv = Vector2D(u, v);
        
        
        // Compute p_dx_uv
        u = (us[0] * coords_dx[0])+(us[1] * coords_dx[1])+(us[2] * coords_dx[2]);
        v = (vs[0] * coords_dx[0])+(vs[1] * coords_dx[1])+(vs[2] * coords_dx[2]);
        // Clip to u v bounds. Ensures Correctness of Bilinear Filtering.
        if (u < 0) u = 0; if (u >= 1) u = 1;
        if (v < 0) v = 0; if (v >= 1) v = 1;
        Vector2D p_dx_uv = p_uv;
        if (dx_pass && !isnan(u) && !isnan(v)) {
          p_dx_uv = Vector2D(u, v);
        }
        
        // Compute p_dy_uv
        u = (us[0] * coords_dy[0])+(us[1] * coords_dy[1])+(us[2] * coords_dy[2]);
        v = (vs[0] * coords_dy[0])+(vs[1] * coords_dy[1])+(vs[2] * coords_dy[2]);
        // Clip to u v bounds. Ensures Correctness of Bilinear Filtering.
        if (u < 0) u = 0; if (u >= 1) u = 1;
        if (v < 0) v = 0; if (v >= 1) v = 1;
        Vector2D p_dy_uv = p_uv;
        if (dy_pass && !isnan(u) && !isnan(v)) {
          p_dy_uv = Vector2D(u, v);
        }
        
        // Sample Color + Render
        sp.p_uv = p_uv; sp.p_dx_uv = p_dx_uv; sp.p_dy_uv = p_dy_uv;
        Color color = tex.sample(sp);
        sample_buffer[j * scale * width + i] = color;
      }
    }
  }

  void RasterizerImp::set_sample_rate(unsigned int rate) {
    // TODO: Task 2: You may want to update this function for supersampling support
    this->sample_rate = rate;
    this->sample_buffer.resize(width * height * rate, Color::White);
    this->clear_buffers();
  }


  void RasterizerImp::set_framebuffer_target(unsigned char* rgb_framebuffer,
    size_t width, size_t height)
  {
    // TODO: Task 2: You may want to update this function for supersampling support
    // Supersampling support is as simple as changing the numbers in the last line.
    // This function is only called when window is resized. aka only time frame buffer is resized.

    this->width = width;
    this->height = height;
    this->rgb_framebuffer_target = rgb_framebuffer;
    this->sample_buffer.resize(width * height * sample_rate, Color::White);
  }


  void RasterizerImp::clear_buffers() {
    std::fill(rgb_framebuffer_target, rgb_framebuffer_target + 3 * width * height, 255);
    std::fill(sample_buffer.begin(), sample_buffer.end(), Color::White);
  }


  // This function is called at the end of rasterizing all elements of the
  // SVG file.  If you use a supersample buffer to rasterize SVG elements
  // for antialising, you could use this call to fill the target framebuffer
  // pixels from the supersample buffer data.
  //
  void RasterizerImp::resolve_to_framebuffer() {
    // TODO: Task 2: You will likely want to update this function for supersampling support
    
    for (int x = 0; x < width; ++x) {
      for (int y = 0; y < height; ++y) {
        
        Color col2 = Color::Black;
        int super_width = (int) sqrt((double) sample_rate);
        int off_x = super_width * x;
        int off_y = super_width * y;
        for (int i = 0; i < super_width; i++) {
          for (int j = 0; j < super_width; j++) {
            Color buf_color = sample_buffer[(j + off_y) * width * super_width + (i + off_x)];
            col2 = col2 + buf_color;
          }
        }
        col2 *= (1.0 / sample_rate);
        Color col = col2;
        
        for (int k = 0; k < 3; ++k) {
          this->rgb_framebuffer_target[3 * (y * width + x) + k] = (&col.r)[k] * 255;
        }
      }
    }

  }

  Rasterizer::~Rasterizer() { }


}// CGL
