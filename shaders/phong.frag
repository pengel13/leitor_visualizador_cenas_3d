#version 460 core

// ============================================================
// phong.frag — Fragment shader implementando o modelo de iluminacao de Phong.
//
// O modelo de Phong decompoe a luz refletida em tres termos:
//
//   AMBIENTE  — iluminacao constante de baixo nivel que simula luz indireta;
//               garante que objetos nunca fiquem completamente pretos.
//
//   DIFUSO    — reflexao Lambertiana: intensidade proporcional ao cosseno
//               do angulo entre a normal da superficie (N) e a direcao da luz (L).
//               max(dot(N,L), 0) garante que faces de tras nao contribuam.
//
//   ESPECULAR — reflexo brilhante. Calcula o vetor de reflexao R de L em torno
//               de N, depois eleva o cosseno do angulo entre R e a direcao da
//               camera V ao expoente 'shininess'. Quanto maior o shininess,
//               menor e mais intenso o highlight.
//
// Resultado combinado:
//   result = atenuacao * (ambiente + difuso + especular)
//
// Atenuacao de luz pontual:
//   att = 1.0 / (constante + linear*d + quadratico*d^2)
//
// Suporte a textura difusa:
//   Se temTextura = true, a cor base vem da amostragem de textureDifusa.
//   Se temTextura = false, usa material_color (definido pelo material .mtl ou JSON).
// ============================================================

// ---- Entradas do vertex shader (interpoladas por fragmento) ----
in vec3 v_fragPos;
in vec3 v_normal;
in vec2 v_texCoords;

// ---- Cor de saida ----
out vec4 fragColor;

// ---- Uniforms de material (enviados por Malha::desenhar(shader)) ----
uniform vec3  material_ambient;    // ka — refletividade ambiente
uniform vec3  material_diffuse;    // kd — refletividade difusa
uniform vec3  material_specular;   // ks — refletividade especular
uniform float material_shininess;  // expoente de Phong (tipicamente 8-256)
uniform vec3  material_color;      // albedo base — usado quando nao ha textura

// ---- Textura difusa (mapa_Kd do .mtl) ----
uniform sampler2D textureDifusa;   // unidade de textura 0
uniform bool      temTextura;      // true: amostrar textureDifusa; false: usar material_color

// ---- Uniforms da fonte de luz pontual (enviados por Renderizador::aplicarLuz) ----
uniform vec3  light_position;
uniform vec3  light_ambient;
uniform vec3  light_diffuse;
uniform vec3  light_specular;
uniform float light_constant;     // atenuacao: termo constante
uniform float light_linear;       // atenuacao: termo linear
uniform float light_quadratic;    // atenuacao: termo quadratico

// ---- Posicao da camera (para calculo do vetor de visao na especular) ----
uniform vec3 viewPos;

// ---- Flag de selecao (adiciona tint sutil ao objeto ativo) ----
uniform bool isSelected;

void main() {
    // Normaliza a normal interpolada (a interpolacao pode encolher seu comprimento)
    vec3 N = normalize(v_normal);

    // Direcao do fragmento para a fonte de luz
    vec3 L = normalize(light_position - v_fragPos);

    // Direcao do fragmento para a camera (observador)
    vec3 V = normalize(viewPos - v_fragPos);

    // Vetor de reflexao da direcao incidente da luz em torno de N.
    // reflect() espera a direcao incidente (da luz para a superficie), por isso negamos L.
    vec3 R = reflect(-L, N);

    // ---- Atenuacao ----
    float dist        = length(light_position - v_fragPos);
    float atenuacao   = 1.0 / (light_constant
                              + light_linear    * dist
                              + light_quadratic * dist * dist);

    // ---- Cor base (albedo) ----
    // Se ha textura difusa, amostra ela; caso contrario usa a cor do material.
    // Isso permite que objetos com .mtl texturizado e sem textura coexistam na cena.
    vec3 corBase = temTextura
                   ? texture(textureDifusa, v_texCoords).rgb
                   : material_color;

    // ---- Componente Ambiente ----
    // Preenchimento constante — mantém faces nao iluminadas visiveis
    vec3 ambiente = light_ambient * material_ambient * corBase;

    // ---- Componente Difuso ----
    // Lei de Lambert: intensidade difusa proporcional a cos(N, L)
    float fatorDif = max(dot(N, L), 0.0);
    vec3  difuso   = light_diffuse * fatorDif * material_diffuse * corBase;

    // ---- Componente Especular ----
    // Highlight de Phong: eleva o cos(V, R) ao expoente shininess.
    // Visivel somente quando a superficie esta voltada para a luz e para o observador.
    float fatorEsp = pow(max(dot(V, R), 0.0), material_shininess);
    vec3  especular = light_specular * fatorEsp * material_specular;

    // ---- Combina com atenuacao ----
    vec3 resultado = (ambiente + difuso + especular) * atenuacao;

    // ---- Tint de selecao ----
    // Adiciona um leve brilho amarelo-branco ao objeto ativo para o usuario saber qual esta selecionado
    if (isSelected) {
        resultado += vec3(0.08, 0.08, 0.02);
    }

    fragColor = vec4(resultado, 1.0);
}
