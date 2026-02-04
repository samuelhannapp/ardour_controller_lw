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
	: wxFrame(nullptr, wxID_ANY, "Hello World")
{
	main_layout = new wxBoxSizer(wxVERTICAL);
	button_layout = new wxBoxSizer(wxHORIZONTAL);
	label_layout = new wxBoxSizer(wxHORIZONTAL);
	fader_layout = new wxBoxSizer(wxHORIZONTAL);

	plugin_list_label = new wxStaticText(this, wxID_ANY, "plugin_list", wxDefaultPosition, wxSize(200, 30));


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
	main_layout->Add(plugin_list_label);
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

//there is a major flaw in the OscMessage modul, before the function GetBytes is called, 
//the address is not part of the data, and after the function GetButes is called, 
//it is part of the data, but the data is wrongly formated, the ',' is part of the types...
//it would be good that at all times, the data would be rightly formated, 
//or another solution would be that there is another field like m_data,
//wich is formated data, and there can be a function called, to initialize this, 
//or it get's updated everytime the message is changed...

//we also need the plugin list here...



void instance::OnSlider(wxCommandEvent& event)
{
	OscMessage osc_message("/wxSlider");
	//thread_message message;
	
	osc_message.PushInt(this->index + (bank * CONTROLLER_SIZE));
	int value = event.GetInt();
	float value_float = float(value) / 1000.0;
	osc_message.PushFloat(value_float);

	int size = 0;
	char *array;
	array = osc_message.GetBytes(size);
	OscMessage osc_message_2(array, size);

	wxThreadEvent event_1 = wxThreadEvent(wxEVT_THREAD); // No specific id
	event_1.SetPayload(osc_message_2);
	wxQueueEvent(handler, event_1.Clone());
}

//we have again here the problem I guess, that we won't get the plugin list, 
//because there is no command like select/plugin/list as request...
//the question here is now wether the strip number of the selected strip is 
//feedbacked, than we could have it...
//but this would not make sense, because we don't have strips here, only the 
//selected one, so we don't need strip numbers, so this message would not be 
//possible, that would mean we need a command like 
//select/plugin/list, wich doesn't exist yet...

//we can use all /strip/ commands!!!! we just have to use as ssid 1...
//that will be the selected than

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
		int plugin_parameter_id_routed = this->plugin_multiplexer->plugin_multiplexer_from_controller[index + CONTROLLER_SIZE * bank];

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

		OscMessage get_plugin_list_message("/strip/plugin/list");
		get_plugin_list_message.PushInt(1);
		int size = 0;
		//get_plugin_list_message.GetBytes(size);
		this->osc_sender_receiver->send_data(get_plugin_list_message);

		this->plugin_multiplexer->setup(plugin_name);
		if (this->selected_plugin_name == plugin_name) //everything is already setup...
			return;
		this->selected_plugin_name = plugin_name;
		
	}

	else if (!osc_message.GetAddress().compare("/strip/plugin/list")) {
		this->plugin_list.clear();
		//write_to_itm(std::to_string(message.GetTypeList().size()));
		int plugin_quantity = (osc_message.GetTypeList().size() - 1) / 3;
		std::string type_list = osc_message.GetTypeList();
		for (int i = 0; i < plugin_quantity; i++)
			this->plugin_list.push_back(osc_message.get_string(i + 2 + i * 2));
		std::string temp_label;
		for (std::string temp : this->plugin_list) {
			temp_label.append(temp);
			temp_label.push_back('\t');
		}
		this->plugin_list_label->SetLabel(temp_label);
		//ardour_sender_receiver.request_plugin_descriptor(local_strip_data.selected_strip.number, local_strip_data.selected_strip.get_selected_plugin_index());
	}
	return;
}
