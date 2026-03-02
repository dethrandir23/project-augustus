#include "../lib/imgui/imgui.h"
#include "../lib/imgui/imgui_impl_glfw.h"
#include "../lib/imgui/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h> // Pencereyi açacak kütüphane
#include <stdio.h>

constexpr const size_t WINDOW_WIDTH = 1280;
constexpr const size_t WINDOW_HEIGHT = 720;
constexpr const char *WINDOW_TITLE = "Deneme";

GLFWwindow *PrepareWindow(size_t width, size_t height, const char *title);
void StartDraw();
void EndDraw(GLFWwindow *window);
void Cleanup(GLFWwindow *window);

float volume = 0.5f;
enum Sayfa {
    SAYFA_DASHBOARD,
    SAYFA_HARITA,
    SAYFA_ENVANTER,
    SAYFA_AYARLAR
};

int main(int, char **) {

  GLFWwindow *window = PrepareWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
  if (!window)
    return 1;
  bool game = true;
  while (!glfwWindowShouldClose(window) && game) {
    StartDraw();
// Aktif sayfa değişkeni (State)
static Sayfa aktif_sayfa = SAYFA_DASHBOARD;

// --- ÇİZİM KISMI ---

// Ekranı kaplayan bir pencere açalım
ImGuiIO &io = ImGui::GetIO();
ImGui::SetNextWindowSize(io.DisplaySize); // Tüm ekranı kapla
ImGui::SetNextWindowPos(ImVec2(0, 0));
ImGui::Begin("Ana Pencere", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);

// 2 Sütunlu Tablo Başlat (Sol: Menü, Sağ: İçerik)
// "SidebarTablosu" -> ID
// 2 -> Sütun sayısı
// ImGuiTableFlags_BordersInnerV -> Ortaya çizgi çek
// ImGuiTableFlags_Resizable -> Kullanıcı ortayı çekiştirebilsin
if (ImGui::BeginTable("SidebarTablosu", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable)) {

    ImGui::TableSetupColumn(
        "Menu",
        ImGuiTableColumnFlags_WidthFixed,
        200.0f
    );
    ImGui::TableSetupColumn(
        "Content",
        ImGuiTableColumnFlags_WidthStretch
    );

    ImGui::TableHeadersRow();

    // --- SOL SÜTUN (MENÜ) ---
    ImGui::TableNextColumn();
    // Genişliği sabitleyebilirsin
    

    ImGui::Text("MENÜ");
    ImGui::Separator();
    
    // Butonlara basınca state değişir
    if (ImGui::Selectable("📊 Dashboard", aktif_sayfa == SAYFA_DASHBOARD)) aktif_sayfa = SAYFA_DASHBOARD;
    if (ImGui::Selectable("🗺️ Harita", aktif_sayfa == SAYFA_HARITA))       aktif_sayfa = SAYFA_HARITA;
    if (ImGui::Selectable("🎒 Envanter", aktif_sayfa == SAYFA_ENVANTER))    aktif_sayfa = SAYFA_ENVANTER;
    if (ImGui::Selectable("⚙️ Ayarlar", aktif_sayfa == SAYFA_AYARLAR))     aktif_sayfa = SAYFA_AYARLAR;

    // --- SAĞ SÜTUN (İÇERİK) ---
    ImGui::TableNextColumn();

    // State neyse onu çiz
    switch (aktif_sayfa) {
        case SAYFA_DASHBOARD:
            ImGui::Text("Hoşgeldin patron. Fabrikalar çalışıyor.");
            break;
        case SAYFA_HARITA:
            ImGui::Button("Harita Yükle"); // Harita fonksiyonunu buraya koycan
            break;
        case SAYFA_ENVANTER:
            ImGui::Text("Cebinde 5 elma var.");
            break;
        case SAYFA_AYARLAR:
            ImGui::SliderFloat("Ses", &volume, 0.0f, 1.0f);
            break;
    }

    ImGui::EndTable();
}

ImGui::End();
EndDraw(window);
  }

  Cleanup(window);
  return 0;
}

GLFWwindow *PrepareWindow(size_t width, size_t height, const char *title) {
  if (!glfwInit())
    return NULL;

  const char *glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE); // for transparency

  GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);
  if (window == NULL)
    return NULL;

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // vsync

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;

  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  ImGui::StyleColorsDark();

  // bind backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  return window;
}

void StartDraw() {
  // input
  glfwPollEvents();

  // prepare new frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void EndDraw(GLFWwindow *window) {
  // -- d. Çizim (Render) --
  ImGui::Render();

  // Ekranı temizle (Arka plan rengi)
  int display_w, display_h;
  glfwGetFramebufferSize(window, &display_w, &display_h);
  glViewport(0, 0, display_w, display_h);
  glClearColor(0.45f, 0.55f, 0.60f, 1.00f); // Mavi-gri bir arka plan
  glClear(GL_COLOR_BUFFER_BIT);

  // ImGui çizimlerini ekrana bas
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  // Tamponları değiştir (Görüntüyü göster)
  glfwSwapBuffers(window);
}

void Cleanup(GLFWwindow *window) {
  printf("1. OpenGL3 kapatiliyor...\n");
  ImGui_ImplOpenGL3_Shutdown();

  printf("2. GLFW Backend kapatiliyor...\n");
  ImGui_ImplGlfw_Shutdown();

  printf("3. Context yikiliyor...\n");
  ImGui::DestroyContext();

  printf("4. Pencere yok ediliyor...\n");
  glfwDestroyWindow(window);

  printf("5. GLFW sonlandiriliyor...\n");
  glfwTerminate();

  printf("6. Bitti. Gecmis olsun.\n");
}