#include "main.hpp"
#include "OscMessage.hpp"
#include "Defines.hpp"

bool MyApp::OnInit()
{
	MyFrame* frame = new MyFrame();
	frame->Show();
	return true;
}

MyFrame::MyFrame()
	: wxFrame(nullptr, wxID_ANY, "General Purpose Controller (gp_controller)")
{
	main_layout = new wxBoxSizer(wxVERTICAL);
	button_layout = new wxBoxSizer(wxHORIZONTAL);
	label_layout = new wxBoxSizer(wxHORIZONTAL);
	fader_layout = new wxBoxSizer(wxHORIZONTAL);

	plugin_name = new wxStaticText(this, wxID_ANY, "plugin_name", wxDefaultPosition, wxSize(200, 30));


	bank_down = new wxButton(this, wxID_ANY, "bank down");
	bank_up = new wxButton(this, wxID_ANY, "bank up");
	plugin_down = new wxButton(this, wxID_ANY, "plugin down");
	plugin_up = new wxButton(this, wxID_ANY, "plugin up");
	bank_down->Bind(wxEVT_BUTTON, &MyFrame::bank_down_function, this);
	bank_up->Bind(wxEVT_BUTTON, &MyFrame::bank_up_function, this);
	plugin_down->Bind(wxEVT_BUTTON, &MyFrame::plugin_down_function, this);
	plugin_up->Bind(wxEVT_BUTTON, &MyFrame::plugin_up_function, this);
	button_layout->Add(bank_down);
	button_layout->Add(bank_up);
	button_layout->Add(plugin_down);
	button_layout->Add(plugin_up);
	
	for (int i = 0; i < 16; i++) {
		controller[i].name = new wxStaticText(this, wxID_ANY, "test", wxDefaultPosition, wxSize(100, 30));
		controller[i].fader = new wxSlider(this, wxID_ANY, 20, 0, 1000, wxDefaultPosition, wxSize(100, 30));
		controller[i].index = i + 1;
		controller[i].handler = (wxEvtHandler*)this;
		label_layout->Add(controller[i].name);
		fader_layout->Add(controller[i].fader);
		controller[i].fader->Bind(wxEVT_SLIDER, &instance::OnSlider, &controller[i]);
	}
	main_layout->Add(plugin_name);
	main_layout->Add(button_layout);
	main_layout->Add(label_layout);
	main_layout->Add(fader_layout);
	this->SetSizerAndFit(main_layout);
	this->osc_sender_receiver = new OscSenderReceiver("127.0.0.1", 16, 3819);
	this->receive_thread = new wxOscReceiveThread(this, this->osc_sender_receiver);
	receive_thread->Run();
	Bind(wxEVT_THREAD, &MyFrame::OnThreadUpdate, this);

	this->plugin_multiplexer = new plugin_multiplexer_struct;
	this->plugin_multiplexer->initialize_plugin_multiplexer();
	this->plugin_multiplexer->initialize_plugin_multiplexer_from_controller_and_from_plugin();

	OscMessage setup_msg("/set_surface");
	setup_msg.PushInt(1);
	setup_msg.PushInt((1 << AudioTracks) | (1 << MidiTracks));
	setup_msg.PushInt(1 << ExtraSelectOnlyFeedback);
	setup_msg.PushInt(0b1000);

	this->osc_sender_receiver->send_data(setup_msg);

}
//these could also all be handled by the thread handler...
void MyFrame::bank_up_function(wxCommandEvent& event)
{
	if (bank < (BANK_SIZE - 1))
		bank++;
	this->update_controller();
}
void MyFrame::bank_down_function(wxCommandEvent& event)
{
	if (bank > 0)
		bank--;
	this->update_controller();
}

void MyFrame::plugin_up_function(wxCommandEvent& event)
{
	OscMessage msg = OscMessage("/select/plugin");
	msg.PushFloat(1);
	this->osc_sender_receiver->send_data(msg);
}

void MyFrame::plugin_down_function(wxCommandEvent& event)
{
	//OscMessage msg = OscMessage("/strip/plugin/list");
	//msg.PushInt(1);
	OscMessage msg = OscMessage("/select/plugin");
	msg.PushFloat(-1);
	this->osc_sender_receiver->send_data(msg);
}

void MyFrame::update_controller()
{
	for (int i = 0; i < CONTROLLER_SIZE; i++) {
		controller[i].name->SetLabel(this->selected_plugin[i + (CONTROLLER_SIZE * bank) + 1].name);
		controller[i].fader->SetValue(int(this->selected_plugin[i + (CONTROLLER_SIZE * bank) + 1].value * float(1000.0)));
	}
}

void instance::OnSlider(wxCommandEvent& event)
{
	OscMessage osc_message("/wxSlider");
	//thread_message message;
	
	osc_message.PushInt(this->index + (bank * CONTROLLER_SIZE));
	int value = event.GetInt();
	float value_float = float(value) / 1000.0;
	osc_message.PushFloat(value_float);

	//this should be automatically done every time something is pushed...
	osc_message.FormatOscMessage(); 

	wxThreadEvent event_1 = wxThreadEvent(wxEVT_THREAD); // No specific id
	event_1.SetPayload(osc_message);
	wxQueueEvent(handler, event_1.Clone());
}

void MyFrame::OnThreadUpdate(wxThreadEvent& event)
{
	// SAFE: runs on GUI thread
	OscMessage osc_message = event.GetPayload<OscMessage>();

	if (!osc_message.GetAddress().compare("/select/plugin/parameter/name")) {
		int plugin_parameter_id = osc_message.get_int(0);
		std::string plugin_parameter_name = osc_message.get_string(1);

		if (plugin_parameter_id >= this->plugin_multiplexer->plugin_multiplexer_from_plugin.size())
			return;	

		int plugin_parameter_id_routed = this->plugin_multiplexer->plugin_multiplexer_from_plugin[plugin_parameter_id];
		this->selected_plugin[plugin_parameter_id_routed].name = plugin_parameter_name;

		if (plugin_parameter_id_routed >= MAX_PLUGIN_PARAMETERS)
			return;

		if (plugin_parameter_id_routed <= 0)
			return;	

		if ((plugin_parameter_id_routed / CONTROLLER_SIZE) == bank)
			this->controller[(plugin_parameter_id_routed - 1) % CONTROLLER_SIZE].name->SetLabel(plugin_parameter_name);
	}
	else if (!osc_message.GetAddress().compare("/select/plugin/parameter")) {
		int plugin_parameter_id = osc_message.get_int(0);
		float plugin_parameter_value = osc_message.initialize_type_list().at(1) == 'f' ? osc_message.get_float(1) : osc_message.get_double(1);
			
		if (plugin_parameter_id >= this->plugin_multiplexer->plugin_multiplexer_from_plugin.size())
			return;	

		int plugin_parameter_id_routed = this->plugin_multiplexer->plugin_multiplexer_from_plugin[plugin_parameter_id];
		this->selected_plugin[plugin_parameter_id_routed].value = plugin_parameter_value;

		if (plugin_parameter_id_routed <= 0)
			return;

		if ((plugin_parameter_id_routed / CONTROLLER_SIZE) == bank )
			this->controller[(plugin_parameter_id_routed - 1) % CONTROLLER_SIZE].fader->SetValue(plugin_parameter_value * 1000);
	}
	else if (!osc_message.GetAddress().compare("/wxSlider")) {
		int index = osc_message.get_int(0);
		float value = (float)osc_message.get_float(1);
		int plugin_parameter_id_routed = this->plugin_multiplexer->plugin_multiplexer_from_controller[index];

		OscMessage osc_message_to_ardour("/select/plugin/parameter");
		osc_message_to_ardour.PushInt(plugin_parameter_id_routed);
		osc_message_to_ardour.PushFloat(value);
		int size = 0;
		osc_message.GetBytes(size);
		this->osc_sender_receiver->send_data(osc_message_to_ardour);
	}
	else if (!osc_message.GetAddress().compare("/select/plugin/name")) {
		std::string plugin_name = osc_message.get_string(0);
		if (plugin_name.size() == 1)
			return;
	
		this->plugin_multiplexer->setup(plugin_name);

		this->plugin_name->SetLabel(plugin_name);
		if (this->selected_plugin_name == plugin_name) //everything is already setup...
			return;
		this->selected_plugin_name = plugin_name;
		
	}
	return;
}
