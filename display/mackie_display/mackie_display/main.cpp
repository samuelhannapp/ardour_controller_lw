#include "main.hpp"

bool MyApp::OnInit()
{
    MyFrame* frame = new MyFrame();
    frame->Show();
    return true;
}


MyFrame::MyFrame()
    : wxFrame(nullptr, wxID_ANY, "Hello World")
{
    wxMenu* menuConnections = new wxMenu;
    menuConnections->Append(ID_Hello, "&connections", "setup ip address for controller");
    menuConnections->Append(ID_OTHER, "&other...\tCtrl+H", "adjust display size to controller");
    menuConnections->AppendSeparator();
    menuConnections->Append(wxID_EXIT);

    wxMenu* menuDisplayFormat = new wxMenu;
    menuDisplayFormat->Append(wxID_ABOUT);

    wxMenu* menuPlugin = new wxMenu;
    menuPlugin->Append(ID_PLUGIN, "save_plugin");

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuConnections, "connections");
    menuBar->Append(menuDisplayFormat, "display format");
    menuBar->Append(menuPlugin, "plugin");

    SetMenuBar(menuBar);

    CreateStatusBar();
    SetStatusText("Welcome to wxWidgets!");

    Bind(wxEVT_MENU, &MyFrame::OnHello, this, ID_Hello);
    Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
}

void MyFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("This is a wxWidgets Hello World example",
        "About Hello World", wxOK | wxICON_INFORMATION);
}

void MyFrame::OnHello(wxCommandEvent& event)
{
    wxLogMessage("Hello world from wxWidgets!");
}