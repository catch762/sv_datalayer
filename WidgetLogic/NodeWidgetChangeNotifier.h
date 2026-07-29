#pragma once
#include "sv_qtcommon.h"

//************************************************************************************************
//
// We, obviously, want to know when widget changes value. And do various things when it does.
// For instance, update DataNode which corresponds to said widget.
// This is mainly done in NodeWidget class, all the details and connections for specific widgets.
//
// But on another, much less specific level, we just want to know when 
// "any existing widget changed its value".
// This is the job of this singleton notifier.
//
//************************************************************************************************

class NodeWidgetChangeNotifier : public QObject
{
	Q_OBJECT
public:
	static QPointer<NodeWidgetChangeNotifier> instance()
	{
		static QPointer<NodeWidgetChangeNotifier> notifier = new NodeWidgetChangeNotifier();
		return notifier;
	}


	// - Disables 'someNodeWidgetChanged' signal
	// - executes 'operation'
	// - Enables signal back and emits it once.
	//
	// This is useful when you have operation which changes lot of widgets at once, but
	// you want to receive just one trigger of 'someNodeWidgetChanged' signal for it.
	static void executeWidgetChangingOperation(const std::function<void()>& operation)
	{
		auto inst = instance();
		SV_ASSERT(inst);

		{
			QSignalBlocker blocker(inst);
			operation();
		}

		emit inst->someNodeWidgetChanged();
	}


signals:
	//Emitted when any NodeWidget existing in entire app changes its value.
	void someNodeWidgetChanged();

private:
	NodeWidgetChangeNotifier() : QObject()
	{
	}

	DISABLE_COPY_AND_ASSIGNMENT(NodeWidgetChangeNotifier);
};