char vertexShaderCode[] = R"(

    //attribute vec2 pos;
    void main()
    {
        //gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
        gl_Position = gl_Vertex;
    }

)";