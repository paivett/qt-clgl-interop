kernel void compute_surface(global float4* vertices, 
                            const float t,
                            const float points_per_side) { 
    size_t x = get_global_id(0);
    size_t y = get_global_id(1);
    size_t i = x*64 + y;
    float4 v = vertices[i];
    float k = M_PI * 2.0f;
    vertices[i].y  = 0.1f * sin(k*(v.x+t))*sin(k*(v.z+t));
}