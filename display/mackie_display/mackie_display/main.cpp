#include "main.hpp"

#define SPACE_SIZE 20

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

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuConnections, "connections");
    menuBar->Append(menuDisplayFormat, "display format");

    SetMenuBar(menuBar);

    //CreateStatusBar();
    //SetStatusText("Welcome to wxWidgets!");

    Bind(wxEVT_MENU, &MyFrame::OnHello, this, ID_Hello);
    Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);

	channel_layout = new wxBoxSizer(wxHORIZONTAL);
	main_layout = new  wxBoxSizer(wxVERTICAL);

	channel_layout->AddSpacer(SPACE_SIZE_SIDE);
	for (int i = 0; i < CHANNEL_COUNT; i++) {
		panel[i] = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
		panel[i]->SetMinSize(wxSize(100, 100));
		panel[i]->SetBackgroundColour(wxColor(210, 210, 210));
		channel_name_panel[i] = new wxPanel(panel[i], wxID_ANY, wxPoint(10, 10) , wxSize(80, 50), wxBORDER_SUNKEN);
		channel_name_panel[i]->SetMinSize(wxSize(80, 50));
		channel_name[i] = new wxStaticText(channel_name_panel[i], wxID_ANY, "test1");
		channel_name[i]->SetBackgroundColour(wxColor(230, 230, 230));
		channel_layout->Add(panel[i], 0, wxSTRETCH_NOT);
		if(i != (CHANNEL_COUNT - 1))
			channel_layout->AddSpacer(SPACE_SIZE);
	}
	channel_layout->AddSpacer(SPACE_SIZE_SIDE);

	main_layout->AddSpacer(SPACE_SIZE);
	main_layout->Add(channel_layout);
	main_layout->AddSpacer(SPACE_SIZE);

	this->SetSizer(main_layout);

	Bind(wxEVT_SIZE, &MyFrame::OnSize, this);

}

void MyFrame::OnSize(wxSizeEvent& event)
{
	wxSize size = event.GetSize();
	wxSize panel_size(size.GetWidth() / 8 - SPACE_SIZE - (SPACE_SIZE_SIDE / 8 * 2), size.GetHeight() - space_size_side); 
	for (int i = 0; i < CHANNEL_COUNT; i++)
		panel[i]->SetMinSize(panel_size);
	this->Update();
	event.Skip();
	return;

}

void MyFrame::OnSlider(wxCommandEvent& event)
{
	space_size_side = event.GetInt();
	wxSizeEvent size_event(wxSize(this->GetSize()));
	this->OnSize(size_event);
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
	WindowScalerFrame *scaler_window = new WindowScalerFrame(this, wxID_ANY, "scaler", wxPoint(100, 100), wxSize(300, 300));
	scaler_window->Show();
	event.Skip();
	return;
}

WindowScalerFrame::WindowScalerFrame(MyFrame *parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size)
	: wxFrame(parent, id, title, pos, size)
{
	wxSlider* slider = new wxSlider(this, wxID_ANY, 40, 0, 500);
	wxBoxSizer* main_layout = new wxBoxSizer(wxHORIZONTAL);
	main_layout->Add(slider);
	this->SetSizer(main_layout);
	slider->Bind(wxEVT_SLIDER, &MyFrame::OnSlider, parent);
}