#pragma once
#include <wx/wx.h>
#include <wx/spinctrl.h>
class MyApp : public wxApp
{
public:
    bool OnInit() override;
};

wxIMPLEMENT_APP(MyApp);

class MyFrame : public wxFrame
{
public:
    MyFrame();
    wxMenu* menuConnections;
    wxMenu* menuDisplayFormat;
    void OnStartArdourController(wxCommandEvent& event);
    void OnStartController(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnAddController(wxCommandEvent& event);
    void PrintMidiDevices();
    void start_process(std::wstring path, std::wstring arguments);

    int get_midi_in(std::string controller_name);
    int get_midi_out(std::string controller_name);
    
    std::vector<std::string>midi_in_devices;
    std::vector<std::string>midi_out_devices;
    int send_port_to_ardour_udp_nr = 3819;

    std::vector<wxPanel*> header_panels;
    std::vector<wxStaticText*> header_lables;

    std::vector<wxChoice*> controllers;
    std::vector<wxTextCtrl*> ardour_network_address;
    std::vector<wxSpinCtrl*> receive_port_from_ardour;
    std::vector<wxSpinCtrl*> send_port_to_ardour;
    std::vector<wxCheckBox*> display_enable_check_box;
    std::vector<wxTextCtrl*> display_network_address;
    std::vector<wxSpinCtrl*> send_port_to_display;
    std::vector<wxSpinCtrl*> receive_port_from_display;
    wxBoxSizer* main_sizer;
    wxBoxSizer* header_sizer;
    std::vector<wxBoxSizer*> controller_layout;
    wxSizer* button_sizer;
    wxButton* add_controller_button;
    wxButton* start_ardour_controller_button;
};