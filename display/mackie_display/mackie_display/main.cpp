#include "main.hpp"
#include "oscmessage.hpp"
#include <vector>

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

	spacers.push_back(channel_layout->AddSpacer(SPACE_SIZE_SIDE));
	for (int i = 0; i < CHANNEL_COUNT; i++) {
		panel[i] = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
		panel[i]->SetMinSize(wxSize(100, 100));
		panel[i]->SetBackgroundColour(wxColor(210, 210, 210));
		channel_name_panel[i] = new wxPanel(panel[i], wxID_ANY, wxPoint(10, 10) , wxSize(80, 50), wxBORDER_SUNKEN);
		channel_name_panel[i]->SetMinSize(wxSize(80, 50));
		channel_name[i] = new wxStaticText(channel_name_panel[i], wxID_ANY, "drums\nmic sm58", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
		wxFont font = channel_name[i]->GetFont();
		font.SetWeight(wxFONTWEIGHT_BOLD);
		font.SetPointSize(10);
		channel_name[i]->SetFont(font);
		channel_name[i]->SetBackgroundColour(wxColor(230, 230, 230));
		channel_layout->Add(panel[i], 0, wxEXPAND);
		if(i != (CHANNEL_COUNT - 1))
			spacers.push_back(channel_layout->AddSpacer(SPACE_SIZE));
	}
	//spacers.push_back(channel_layout->AddSpacer(SPACE_SIZE_SIDE));

	top_spacer = main_layout->AddSpacer(SPACE_SIZE);
	main_layout->Add(channel_layout);
	main_layout->AddSpacer(SPACE_SIZE);

	this->SetSizer(main_layout);
	this->osc_sender_receiver = new OscSenderReceiver("127.0.0.1", 12, 11);
	Bind(wxEVT_SIZE, &MyFrame::OnSize, this);
	thread = new wxOscReceiveThread(this, osc_sender_receiver);
	thread->Run();
	Bind(wxEVT_THREAD, &MyFrame::OnThreadUpdate, this);
}

void MyFrame::OnSize(wxSizeEvent& event)
{
	wxSize size = event.GetSize();
	int spacer_count = this->spacers.size();
	wxSize panel_size;
	panel_size = wxSize(size.GetWidth() / 8 - SPACE_SIZE - (space_size_side), size.GetHeight() - MENU_SIZE - SPACE_SIZE - panel_offset);
	spacers[0]->SetMinSize(wxSize(left_spacer_size, 0));
	top_spacer->SetMinSize(wxSize(0, panel_offset));
	
	wxSize channel_name_size(panel_size.GetWidth() - SPACE_SIZE * 2, CHANNEL_NAME_HEIGHT);
	for (int i = 0; i < CHANNEL_COUNT; i++) {
		panel[i]->SetMinSize(panel_size);
		channel_name_panel[i]->SetSize(channel_name_size);
		channel_name_panel[i]->SetPosition(wxPoint(SPACE_SIZE, size.GetHeight() - CHANNEL_NAME_HEIGHT - MENU_SIZE - SPACE_SIZE * 2 - panel_offset));
		channel_name[i]->SetSize(channel_name_size);
	}

	this->Update();
	event.Skip();
	return;
}

void MyFrame::OnSlider(wxCommandEvent& event)
{
	space_size_side = event.GetInt() / 14;
	wxSize original_size = this->GetSize();
	wxSize temp_size(600, 600);
	this->SetSize(temp_size);
	this->SetSize(original_size);
}

void MyFrame::OnSlider_2(wxCommandEvent& event)
{
	left_spacer_size = event.GetInt() / 3;
	wxSize original_size = this->GetSize();
	wxSize temp_size(600, 600);
	this->SetSize(temp_size);
	this->SetSize(original_size);
}

void MyFrame::OnSlider_3(wxCommandEvent& event)
{
	panel_offset = event.GetInt();
	wxSize original_size = this->GetSize();
	wxSize temp_size(600, 600);
	this->SetSize(temp_size);
	this->SetSize(original_size);
}

void MyFrame::OnThreadUpdate(wxThreadEvent& event)
{
	// SAFE: runs on GUI thread
	OscMessage message = event.GetPayload<OscMessage>();
	int index = message.get_int(0) - 1;
	this->channel_name[index]->SetLabel(message.get_string(1));
	this->channel_name[index]->SetSize(channel_name_panel[index]->GetSize());
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
	wxSlider* slider_1 = new wxSlider(this, wxID_ANY, SPACE_SIZE, SPACE_SIZE, 800);
	wxSlider* slider_2 = new wxSlider(this, wxID_ANY, SPACE_SIZE, SPACE_SIZE, 800);
	wxSlider* slider_3 = new wxSlider(this, wxID_ANY, SPACE_SIZE, SPACE_SIZE, 800);
	wxBoxSizer* slider_layout = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* label_layout = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* main_layout = new wxBoxSizer(wxVERTICAL);
	slider_value_1 = new wxStaticText(this, wxID_ANY, "value");
	slider_value_2 = new wxStaticText(this, wxID_ANY, "value");
	slider_value_3 = new wxStaticText(this, wxID_ANY, "value");
	
	slider_layout->Add(slider_1);
	slider_layout->Add(slider_2);
	slider_layout->Add(slider_3);

	label_layout->Add(slider_value_1);
	label_layout->Add(slider_value_2);
	label_layout->Add(slider_value_3);

	main_layout->Add(slider_layout);
	main_layout->Add(label_layout);

	this->SetSizer(main_layout);

	slider_1->Bind(wxEVT_SLIDER, &MyFrame::OnSlider, parent);
	slider_2->Bind(wxEVT_SLIDER, &MyFrame::OnSlider_2, parent);
	slider_3->Bind(wxEVT_SLIDER, &MyFrame::OnSlider_3, parent);

	slider_1->Bind(wxEVT_SLIDER, &WindowScalerFrame::OnSlider_1, this);
	slider_2->Bind(wxEVT_SLIDER, &WindowScalerFrame::OnSlider_2, this);
	slider_3->Bind(wxEVT_SLIDER, &WindowScalerFrame::OnSlider_3, this);
}

void WindowScalerFrame::OnSlider_1(wxCommandEvent& event)
{
	this->slider_value_1->SetLabel(std::to_string(event.GetInt()));
	event.Skip();
}

void WindowScalerFrame::OnSlider_2(wxCommandEvent& event)
{
	this->slider_value_2->SetLabel(std::to_string(event.GetInt()));
	event.Skip();
}

void WindowScalerFrame::OnSlider_3(wxCommandEvent& event)
{
	this->slider_value_3->SetLabel(std::to_string(event.GetInt()));
	event.Skip();
}