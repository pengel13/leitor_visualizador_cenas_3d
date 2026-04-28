#version 460 core

// ============================================================
// phong.frag — Fragment shader implementing the Phong reflection model.
//
// The Phong model decomposes reflected light into three terms:
//
//   AMBIENT  — constant low-level illumination simulating indirect
//              bounced light; ensures objects are never pitch black.
//
//   DIFFUSE  — Lambertian reflection: intensity proportional to the
//              cosine of the angle between the surface normal (N) and
//              the light direction (L).  max(dot(N,L), 0) clamps to
//              avoid negative contributions from behind-the-surface light.
//
//   SPECULAR — Mirror-like highlight.  We compute the reflection
//              direction R of L about N, then raise the cosine of the
//              angle between R and the view direction V to the power of
//              'shininess'.  Higher shininess = smaller, tighter highlight.
//
// Combined:
//   result = attenuation * (ambient + diffuse + specular)
//
// Point light attenuation:
//   att = 1.0 / (constant + linear*d + quadratic*d^2)
//   This physically models how light intensity falls off with distance.
// ============================================================

// ---- Inputs from vertex shader (interpolated per-fragment) ----
in vec3 v_fragPos;
in vec3 v_normal;
in vec2 v_texCoords;

// ---- Output colour ----
out vec4 fragColor;

// ---- Material uniforms (uploaded by Renderer::applyMaterial) ----
uniform vec3  material_ambient;    // ka — ambient  reflectivity
uniform vec3  material_diffuse;    // kd — diffuse  reflectivity
uniform vec3  material_specular;   // ks — specular reflectivity
uniform float material_shininess;  // Phong exponent (typically 8 – 256)
uniform vec3  material_color;      // base albedo colour (multiplied with kd)

// ---- Point light uniforms (uploaded by Renderer::applyLight) ----
uniform vec3  light_position;
uniform vec3  light_ambient;
uniform vec3  light_diffuse;
uniform vec3  light_specular;
uniform float light_constant;      // attenuation: constant term
uniform float light_linear;        // attenuation: linear term
uniform float light_quadratic;     // attenuation: quadratic term

// ---- Camera position (for specular view-direction calculation) ----
uniform vec3  viewPos;

// ---- Selection flag (adds a subtle highlight to the active object) ----
uniform bool  isSelected;

void main() {
    // Normalise the interpolated normal (interpolation can shrink its length)
    vec3 N = normalize(v_normal);

    // Direction from fragment to light source
    vec3 L = normalize(light_position - v_fragPos);

    // Direction from fragment to camera (viewer)
    vec3 V = normalize(viewPos - v_fragPos);

    // Reflection of the incoming light direction about the surface normal.
    // reflect() expects the incident direction (i.e., from light to surface),
    // so we negate L.
    vec3 R = reflect(-L, N);

    // ---- Attenuation ----
    // Distance from fragment to the light
    float dist = length(light_position - v_fragPos);

    // Classic quadratic attenuation formula
    float attenuation = 1.0 / (light_constant
                              + light_linear    * dist
                              + light_quadratic * dist * dist);

    // ---- Ambient ----
    // Constant fill light — keeps unlit surfaces from being fully black
    vec3 ambient = light_ambient * material_ambient * material_color;

    // ---- Diffuse ----
    // Lambert's cosine law: diffuse intensity ∝ cos(N, L) = dot(N, L)
    // clamp to 0 so backfacing fragments get no diffuse contribution
    float diffFactor = max(dot(N, L), 0.0);
    vec3  diffuse    = light_diffuse * diffFactor * material_diffuse * material_color;

    // ---- Specular ----
    // Phong specular: raise the cos(V, R) to the shininess exponent.
    // Only visible when the surface faces both the light and the viewer.
    float specFactor = pow(max(dot(V, R), 0.0), material_shininess);
    vec3  specular   = light_specular * specFactor * material_specular;

    // ---- Combine with attenuation ----
    vec3 result = (ambient + diffuse + specular) * attenuation;

    // ---- Selection tint ----
    // Add a faint yellow-white rim to the currently selected object
    // so the user knows which one is active.
    if (isSelected) {
        result += vec3(0.08, 0.08, 0.02);
    }

    fragColor = vec4(result, 1.0);
}
