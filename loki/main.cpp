#include <math.h>
#include <imgui.h>
#include "imgui_impl_glfw_gl3.h"
#include <stdio.h>
#include <stdlib.h>
#include <GL/gl3w.h>    // This example is using gl3w to access OpenGL functions (because it is small). You may use glew/glad/glLoadGen/etc. whatever already works for you.
#include <GLFW/glfw3.h>

static void error_callback(int error, const char* description)
{
  fprintf(stderr, "Error %d: %s\n", error, description);
}


// NB: You can use math functions/operators on ImVec2 if you #define IMGUI_DEFINE_MATH_OPERATORS and #include "imgui_internal.h"
// Here we only declare simple +/- operators so others don't leak into the demo code.
static inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
static inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }

static bool show_mesh_tools = false;
static bool show_texture_tools = false;
static bool show_animation_tools = true;
static bool show_shader_tools = true;

static void ShowExampleMenuFile()
{ 
  //ImGui::Separator();

  if (ImGui::MenuItem("Quit", "Alt+F4")) { exit(0); }
}

static void ShowExampleAppMainMenuBar()
{
  if (ImGui::BeginMainMenuBar())
  {
    if (ImGui::BeginMenu("File"))
    {
      ShowExampleMenuFile();
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit"))
    {
      if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
      if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
      ImGui::Separator();
      if (ImGui::MenuItem("Cut", "CTRL+X")) {}
      if (ImGui::MenuItem("Copy", "CTRL+C")) {}
      if (ImGui::MenuItem("Paste", "CTRL+V")) {}
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View"))
    {
      if (ImGui::MenuItem("Animation Tools", NULL, show_animation_tools)) { show_animation_tools ^= 1; }
      if (ImGui::MenuItem("Shader Tools", NULL, show_shader_tools)) { show_shader_tools ^= 1; }
      if (ImGui::MenuItem("Mesh Tools", NULL, show_mesh_tools)) { show_mesh_tools ^= 1; }
      if (ImGui::MenuItem("Texture Tools", NULL, show_texture_tools)) { show_texture_tools ^= 1; }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
}

// Really dumb data structure provided for the example.
// Note that we storing links are INDICES (not ID) to make example code shorter, obviously a bad idea for any general purpose code.
static void ShowExampleAppCustomNodeGraph(bool* opened)
{
  ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiSetCond_FirstUseEver);
  if (!ImGui::Begin("Shader Tools", opened, ImGuiWindowFlags_MenuBar))
  {
    ImGui::End();
    return;
  }

  if (ImGui::BeginMenuBar())
  {
    if (ImGui::BeginMenu("File"))
    {
      if (ImGui::MenuItem("Open Shader...", NULL, false))
      {

      }
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }

  // Dummy
  struct Node
  {
    int     ID;
    char    Name[32];
    ImVec2  Pos, Size;
    float   Value;
    ImVec4  Color;
    int     InputsCount, OutputsCount;

    Node(int id, const char* name, const ImVec2& pos, float value, const ImVec4& color, int inputs_count, int outputs_count) { ID = id; strncpy(Name, name, 31); Name[31] = 0; Pos = pos; Value = value; Color = color; InputsCount = inputs_count; OutputsCount = outputs_count; }

    ImVec2 GetInputSlotPos(int slot_no) const { return ImVec2(Pos.x, Pos.y + Size.y * ((float)slot_no + 1) / ((float)InputsCount + 1)); }
    ImVec2 GetOutputSlotPos(int slot_no) const { return ImVec2(Pos.x + Size.x, Pos.y + Size.y * ((float)slot_no + 1) / ((float)OutputsCount + 1)); }
  };
  struct NodeLink
  {
    int     InputIdx, InputSlot, OutputIdx, OutputSlot;

    NodeLink(int input_idx, int input_slot, int output_idx, int output_slot) { InputIdx = input_idx; InputSlot = input_slot; OutputIdx = output_idx; OutputSlot = output_slot; }
  };

  static ImVector<Node> nodes;
  static ImVector<NodeLink> links;
  static bool inited = false;
  static ImVec2 scrolling = ImVec2(0.0f, 0.0f);
  static bool show_grid = true;
  static int node_selected = -1;
  if (!inited)
  {
    nodes.push_back(Node(0, "MainTex", ImVec2(40, 50), 0.5f, ImColor(255, 100, 100), 1, 1));
    nodes.push_back(Node(1, "BumpMap", ImVec2(40, 150), 0.42f, ImColor(200, 100, 200), 1, 1));
    nodes.push_back(Node(2, "Combine", ImVec2(270, 80), 1.0f, ImColor(0, 200, 100), 2, 2));
    links.push_back(NodeLink(0, 0, 2, 0));
    links.push_back(NodeLink(1, 0, 2, 1));
    inited = true;
  }

  // Draw a list of nodes on the left side
  bool open_context_menu = false;
  int node_hovered_in_list = -1;
  int node_hovered_in_scene = -1;
  ImGui::BeginChild("node_list", ImVec2(100, 0));
  ImGui::Text("Nodes");
  ImGui::Separator();
  for (int node_idx = 0; node_idx < nodes.Size; node_idx++)
  {
    Node* node = &nodes[node_idx];
    ImGui::PushID(node->ID);
    if (ImGui::Selectable(node->Name, node->ID == node_selected))
      node_selected = node->ID;
    if (ImGui::IsItemHovered())
    {
      node_hovered_in_list = node->ID;
      open_context_menu |= ImGui::IsMouseClicked(1);
    }
    ImGui::PopID();
  }
  ImGui::EndChild();

  ImGui::SameLine();
  ImGui::BeginGroup();

  const float NODE_SLOT_RADIUS = 4.0f;
  const ImVec2 NODE_WINDOW_PADDING(8.0f, 8.0f);

  // Create our child canvas
  ImGui::Text("Hold middle mouse button to scroll (%.2f,%.2f)", scrolling.x, scrolling.y);
  ImGui::SameLine(ImGui::GetWindowWidth() - 100);
  ImGui::Checkbox("Show grid", &show_grid);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImColor(60, 60, 70, 200));
  ImGui::BeginChild("scrolling_region", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
  ImGui::PushItemWidth(120.0f);

  ImVec2 offset = ImGui::GetCursorScreenPos() - scrolling;
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  draw_list->ChannelsSplit(2);

  // Display grid
  if (show_grid)
  {
    ImU32 GRID_COLOR = ImColor(200, 200, 200, 40);
    float GRID_SZ = 64.0f;
    ImVec2 win_pos = ImGui::GetCursorScreenPos();
    ImVec2 canvas_sz = ImGui::GetWindowSize();
    for (float x = fmodf(offset.x, GRID_SZ); x < canvas_sz.x; x += GRID_SZ)
      draw_list->AddLine(ImVec2(x, 0.0f) + win_pos, ImVec2(x, canvas_sz.y) + win_pos, GRID_COLOR);
    for (float y = fmodf(offset.y, GRID_SZ); y < canvas_sz.y; y += GRID_SZ)
      draw_list->AddLine(ImVec2(0.0f, y) + win_pos, ImVec2(canvas_sz.x, y) + win_pos, GRID_COLOR);
  }

  // Display links
  draw_list->ChannelsSetCurrent(0); // Background
  for (int link_idx = 0; link_idx < links.Size; link_idx++)
  {
    NodeLink* link = &links[link_idx];
    Node* node_inp = &nodes[link->InputIdx];
    Node* node_out = &nodes[link->OutputIdx];
    ImVec2 p1 = offset + node_inp->GetOutputSlotPos(link->InputSlot);
    ImVec2 p2 = offset + node_out->GetInputSlotPos(link->OutputSlot);
    draw_list->AddBezierCurve(p1, p1 + ImVec2(+50, 0), p2 + ImVec2(-50, 0), p2, ImColor(200, 200, 100), 3.0f);
  }

  // Display nodes
  for (int node_idx = 0; node_idx < nodes.Size; node_idx++)
  {
    Node* node = &nodes[node_idx];
    ImGui::PushID(node->ID);
    ImVec2 node_rect_min = offset + node->Pos;

    // Display node contents first
    draw_list->ChannelsSetCurrent(1); // Foreground
    bool old_any_active = ImGui::IsAnyItemActive();
    ImGui::SetCursorScreenPos(node_rect_min + NODE_WINDOW_PADDING);
    ImGui::BeginGroup(); // Lock horizontal position
    ImGui::Text("%s", node->Name);
    ImGui::SliderFloat("##value", &node->Value, 0.0f, 1.0f, "Alpha %.2f");
    ImGui::ColorEdit3("##color", &node->Color.x);
    ImGui::EndGroup();

    // Save the size of what we have emitted and whether any of the widgets are being used
    bool node_widgets_active = (!old_any_active && ImGui::IsAnyItemActive());
    node->Size = ImGui::GetItemRectSize() + NODE_WINDOW_PADDING + NODE_WINDOW_PADDING;
    ImVec2 node_rect_max = node_rect_min + node->Size;

    // Display node box
    draw_list->ChannelsSetCurrent(0); // Background
    ImGui::SetCursorScreenPos(node_rect_min);
    ImGui::InvisibleButton("node", node->Size);
    if (ImGui::IsItemHovered())
    {
      node_hovered_in_scene = node->ID;
      open_context_menu |= ImGui::IsMouseClicked(1);
    }
    bool node_moving_active = ImGui::IsItemActive();
    if (node_widgets_active || node_moving_active)
      node_selected = node->ID;
    if (node_moving_active && ImGui::IsMouseDragging(0))
      node->Pos = node->Pos + ImGui::GetIO().MouseDelta;

    ImU32 node_bg_color = (node_hovered_in_list == node->ID || node_hovered_in_scene == node->ID || (node_hovered_in_list == -1 && node_selected == node->ID)) ? ImColor(75, 75, 75) : ImColor(60, 60, 60);
    draw_list->AddRectFilled(node_rect_min, node_rect_max, node_bg_color, 4.0f);
    draw_list->AddRect(node_rect_min, node_rect_max, ImColor(100, 100, 100), 4.0f);
    for (int slot_idx = 0; slot_idx < node->InputsCount; slot_idx++)
      draw_list->AddCircleFilled(offset + node->GetInputSlotPos(slot_idx), NODE_SLOT_RADIUS, ImColor(150, 150, 150, 150));
    for (int slot_idx = 0; slot_idx < node->OutputsCount; slot_idx++)
      draw_list->AddCircleFilled(offset + node->GetOutputSlotPos(slot_idx), NODE_SLOT_RADIUS, ImColor(150, 150, 150, 150));

    ImGui::PopID();
  }
  draw_list->ChannelsMerge();

  // Open context menu
  if (!ImGui::IsAnyItemHovered() && ImGui::IsMouseHoveringWindow() && ImGui::IsMouseClicked(1))
  {
    node_selected = node_hovered_in_list = node_hovered_in_scene = -1;
    open_context_menu = true;
  }
  if (open_context_menu)
  {
    ImGui::OpenPopup("context_menu");
    if (node_hovered_in_list != -1)
      node_selected = node_hovered_in_list;
    if (node_hovered_in_scene != -1)
      node_selected = node_hovered_in_scene;
  }

  // Draw context menu
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
  if (ImGui::BeginPopup("context_menu"))
  {
    Node* node = node_selected != -1 ? &nodes[node_selected] : NULL;
    ImVec2 scene_pos = ImGui::GetMousePosOnOpeningCurrentPopup() - offset;
    if (node)
    {
      ImGui::Text("Node '%s'", node->Name);
      ImGui::Separator();
      if (ImGui::MenuItem("Rename..", NULL, false, false)) {}
      if (ImGui::MenuItem("Delete", NULL, false, false)) {}
      if (ImGui::MenuItem("Copy", NULL, false, false)) {}
    }
    else
    {
      if (ImGui::MenuItem("Add")) { nodes.push_back(Node(nodes.Size, "New node", scene_pos, 0.5f, ImColor(100, 100, 200), 2, 2)); }
      if (ImGui::MenuItem("Paste", NULL, false, false)) {}
    }
    ImGui::EndPopup();
  }
  ImGui::PopStyleVar();

  // Scrolling
  if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(2, 0.0f))
    scrolling = scrolling - ImGui::GetIO().MouseDelta;

  ImGui::PopItemWidth();
  ImGui::EndChild();
  ImGui::PopStyleColor();
  ImGui::PopStyleVar(2);
  ImGui::EndGroup();

  ImGui::End();
}

void render_mesh()
{
  //render the currently loaded mesh to our FBO
}

int main(int argc, char **argv)
{
  // Setup window
  glfwSetErrorCallback(error_callback);
  if (!glfwInit())
    return 1;
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
  GLFWwindow* window = glfwCreateWindow(1280, 720, "Loki", NULL, NULL);
  glfwMakeContextCurrent(window);
  gl3wInit();

  // Setup ImGui binding
  ImGui_ImplGlfwGL3_Init(window, true);

  // Load Fonts
  // (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
  //ImGuiIO& io = ImGui::GetIO();
  //io.Fonts->AddFontDefault();
  //io.Fonts->AddFontFromFileTTF("../../extra_fonts/Cousine-Regular.ttf", 15.0f);
  //io.Fonts->AddFontFromFileTTF("../../extra_fonts/DroidSans.ttf", 16.0f);
  //io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyClean.ttf", 13.0f);
  //io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyTiny.ttf", 10.0f);
  //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());

  bool show_test_window = true;
  bool show_another_window = false;
  ImVec4 clear_color = ImColor(114, 144, 154);

  while (!glfwWindowShouldClose(window))
  {
    glfwPollEvents();
    ImGui_ImplGlfwGL3_NewFrame();

    ShowExampleAppMainMenuBar();

    if (show_shader_tools)
    {
      ShowExampleAppCustomNodeGraph(&show_shader_tools);
    }
    
    if (show_animation_tools)
    {
      ImGui::SetNextWindowSize(ImVec2(100, 100), ImGuiSetCond_FirstUseEver);
      ImGui::Begin("Animation Tools", &show_animation_tools, ImGuiWindowFlags_MenuBar);
      ImGui::Text("");

      if (ImGui::BeginMenuBar())
      {
        if (ImGui::BeginMenu("File"))
        {
          if (ImGui::MenuItem("Open Animation...", NULL, false))
          {

          }
          ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
      }

      ImGui::End();
    }

    if (show_mesh_tools)
    {
      ImGui::SetNextWindowSize(ImVec2(100, 100), ImGuiSetCond_FirstUseEver);
      ImGui::Begin("Mesh Tools", &show_mesh_tools, ImGuiWindowFlags_MenuBar);
      ImGui::Text("");

      if (ImGui::BeginMenuBar())
      {
        if (ImGui::BeginMenu("File"))
        {
          if (ImGui::MenuItem("Open Mesh...", NULL, false))
          {

          }
          ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
      }

      //render the mesh
      //TODO: grab tex_id from FBO & render mesh to FBO
      ImTextureID tex_id = ImGui::GetIO().Fonts->TexID;
      ImGui::Image(tex_id, ImVec2(100, 100), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));

      ImGui::End();
    }

    if (show_texture_tools)
    {
      ImGui::SetNextWindowSize(ImVec2(100, 100), ImGuiSetCond_FirstUseEver);
      ImGui::Begin("Texture Tools", &show_texture_tools, ImGuiWindowFlags_MenuBar);
      ImGui::Text("");

      if (ImGui::BeginMenuBar())
      {
        if (ImGui::BeginMenu("File"))
        {
          if (ImGui::MenuItem("Open Texture...", NULL, false))
          {

          }
          ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
      }

      //render the texture
      //TODO: grab tex_id from FBO & render mesh to FBO
      ImTextureID tex_id = ImGui::GetIO().Fonts->TexID;
      ImGui::Image(tex_id, ImVec2(100, 100), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));


      ImGui::End();
    }

    if (show_mesh_tools)
    {
      ImGui::SetNextWindowSize(ImVec2(100, 100), ImGuiSetCond_FirstUseEver);
      ImGui::Begin("Mesh Tools", &show_mesh_tools);
      ImGui::Text("");
      ImGui::End();
    }

    /*
    ImGui::SetNextWindowSize(ImVec2(100, 100), ImGuiSetCond_FirstUseEver);
    ImGui::Begin("Texture2D", &show_another_window);
    ImGui::Text("");
    ImGui::End();
    */

    // Rendering
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui::Render();
    glfwSwapBuffers(window);
  }

  // Main loop
  /*
  while (!glfwWindowShouldClose(window))
  {
    glfwPollEvents();
    ImGui_ImplGlfwGL3_NewFrame();

    // 1. Show a simple window
    // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
    {
      static float f = 0.0f;
      ImGui::Text("Hello, world!");
      ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
      ImGui::ColorEdit3("clear color", (float*)&clear_color);
      if (ImGui::Button("Test Window")) show_test_window ^= 1;
      if (ImGui::Button("Another Window")) show_another_window ^= 1;
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    }

    // 2. Show another simple window, this time using an explicit Begin/End pair
    if (show_another_window)
    {
      ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
      ImGui::Begin("Another Window", &show_another_window);
      ImGui::Text("Hello");
      ImGui::End();
    }

    // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
    if (show_test_window)
    {
      ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
      ImGui::ShowTestWindow(&show_test_window);
    }

    // Rendering
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui::Render();
    glfwSwapBuffers(window);
  }
  */

  // Cleanup
  ImGui_ImplGlfwGL3_Shutdown();
  glfwTerminate();

  return 0;
}
/*
#include <gtk/gtk.h>

static void activate(GtkApplication* app, gpointer user_data)
{
  GtkWidget *window;

  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Window");
  gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
  gtk_widget_show_all(window);
}

int main(int argc, char **argv)
{
  GtkApplication *app;
  int status;

  app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}
*/