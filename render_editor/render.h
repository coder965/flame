#include <sstream>

#include <qtreewidget.h>
#include <qlistwidget.h>
#include <qlayout.h>
#include "../src/core/qt.h"

extern QTreeWidget *tree;

void setAddButton(QToolButton *b);

void setRemoveButton(QToolButton *b);

void setUpButton(QToolButton *b);

void setDownButton(QToolButton *b);

struct Drawcall : tke::DrawcallAbstract
{
	struct Wrap : QObject
	{
		Drawcall *p;
		tke::QTreeItemPair<QToolButton> item;
		void appear(Drawcall *_p, QTreeWidgetItem *parent, std::string *name)
		{
			p = _p;

			item.partner = new QToolButton;
			setRemoveButton(item.partner);
			item.setup(tree, parent, name->c_str(), name);
			connect(item.partner, &QToolButton::clicked, this, &Wrap::remove);

			for (auto r : p->b->reflectons)
			{
				if (r->name == "name")
					continue;
				tke::qAddTreeItem(tree, item.item, p, r);
			}
		}
		void remove()
		{
			p->mark = eMarkClear;
			p->parent->maintain(0);
		}
	}wrap;

	void appear(QTreeWidgetItem *parent)
	{
		wrap.appear(this, parent, &name);
	}
};

struct DrawAction : tke::DrawActionAbstract<Drawcall>
{
	struct Wrap : QObject
	{
		DrawAction *p;
		tke::QTreeItemPair<QToolButton> item;
		tke::QTreeItemPair<tke::QIntDropCombo> typeItem;
		tke::QTreeItemPair<QToolButton> drawcallsItem;
		void appear(DrawAction *_p, QTreeWidgetItem *parent, std::string *name)
		{
			p = _p;

			item.partner = new QToolButton;
			setRemoveButton(item.partner);
			item.setup(tree, parent, name->c_str(), name);
			connect(item.partner, &QToolButton::clicked, this, &Wrap::remove);

			for (auto r : p->b->reflectons)
			{
				if (r->name == "name")
					continue;
				tke::qAddTreeItem(tree, item.item, p, r);
			}

			drawcallsItem.partner = new QToolButton;
			setAddButton(drawcallsItem.partner);
			drawcallsItem.setup(tree, item.item, "Drawcalls");
			connect(drawcallsItem.partner, &QToolButton::clicked, this, &Wrap::newDrawcall);

			for (auto &drawcall : p->drawcalls)
				drawcall.appear(drawcallsItem.item);
		}
		void remove()
		{
			p->mark = eMarkClear;
			p->parent->maintain(0);
		}
		void newDrawcall()
		{
			auto drawcall = p->addDrawcall();
			std::stringstream name;
			name << p->drawcalls.size();
			drawcall->name = name.str();
			drawcall->appear(drawcallsItem.item);
		}
	}wrap;

	void appear(QTreeWidgetItem *parent)
	{
		wrap.appear(this, parent, &name);
	}
};

struct Attachment : tke::AttachmentAbstract
{
	struct Wrap : QObject
	{
		int type;
		Attachment *p;
		tke::QTreeItemPair<QToolButton> item;
		void appear(Attachment *_p, QTreeWidgetItem *parent, int _type, std::string *name)
		{
			p = _p;
			type = _type;

			item.partner = new QToolButton;
			setRemoveButton(item.partner);
			item.setup(tree, parent, name->c_str(), name);
			connect(item.partner, &QToolButton::clicked, this, &Wrap::remove);

			for (auto r : p->b->reflectons)
			{
				if (r->name == "name")
					continue;
				tke::qAddTreeItem(tree, item.item, p, r);
			}
		}
		void remove()
		{
			p->mark = Element::eMarkClear;
			p->parent->maintain(type == 0 ? (int)tke::RenderPassElement::eColorAttachment : (int)tke::RenderPassElement::eDepthStencilAttachment);
		}
	}wrap;

	void appear(QTreeWidgetItem *parent, int _type)
	{
		wrap.appear(this, parent, _type, &name);
	}
};

struct Dependency : tke::DependencyAbstract
{
	struct Wrap : QObject
	{
		Dependency *p;
		tke::QTreeItemPair<QToolButton> item;
		void appear(Dependency *_p, QTreeWidgetItem *parent, std::string *pass_name)
		{
			p = _p;

			item.partner = new QToolButton;
			setRemoveButton(item.partner);
			item.setup(tree, parent, pass_name->c_str(), pass_name);
			connect(item.partner, &QToolButton::clicked, this, &Wrap::remove);
		}
		void remove()
		{
			p->mark = tke::Element::eMarkClear;
			p->parent->maintain(0);
		}
	}wrap;

	void appear(QTreeWidgetItem *parent)
	{
		wrap.appear(this, parent, &pass_name);
	}

};

struct RenderPass : tke::RenderPassAbstract<Attachment, Dependency, DrawAction>
{
	struct Wrap : QObject
	{
		RenderPass *p;
		int index;
		tke::QTreeItemPair<QGroupBox> item;
		QTreeWidgetItem *attachmentsItem = nullptr;
		tke::QTreeItemPair<QToolButton> colorsItem;
		tke::QTreeItemPair<QToolButton> depthStencilItem;
		tke::QTreeItemPair<QToolButton> dependenciesItem;
		tke::QTreeItemPair<QToolButton> actionsItem;
		void appear(RenderPass *_p, QTreeWidgetItem *parent, std::string *name)
		{
			p = _p;
			auto removeButton = new QToolButton;
			setRemoveButton(removeButton);
			auto upButton = new QToolButton;
			setUpButton(upButton);
			auto downButton = new QToolButton;
			setDownButton(downButton);

			auto layout = new QHBoxLayout;
			layout->addWidget(removeButton);
			layout->addWidget(upButton);
			layout->addWidget(downButton);
			layout->setAlignment(Qt::AlignLeft);

			item.partner = new QGroupBox;
			item.partner->setLayout(layout);
			item.partner->setStyleSheet("QGroupBox{border:0px;padding-left:-11px;padding-top:-10px;padding-bottom:-10px;}");
			item.setup(tree, parent, name->c_str(), name);
			connect(removeButton, &QToolButton::clicked, this, &Wrap::remove);
			connect(upButton, &QToolButton::clicked, this, &Wrap::up);
			connect(downButton, &QToolButton::clicked, this, &Wrap::down);

			for (auto r : p->b->reflectons)
			{
				if (r->name == "name")
					continue;
				tke::qAddTreeItem(tree, item.item, p, r);
			}

			attachmentsItem = new QTreeWidgetItem(item.item, QStringList("Attachments"));

			colorsItem.partner = new QToolButton;
			setAddButton(colorsItem.partner);
			colorsItem.setup(tree, attachmentsItem, "Color");
			connect(colorsItem.partner, &QToolButton::clicked, this, &Wrap::newColorAttachment);
			for (auto &color : p->colorAttachments)
				color.appear(colorsItem.item, 0);

			depthStencilItem.partner = new QToolButton;
			setAddButton(depthStencilItem.partner);
			depthStencilItem.setup(tree, attachmentsItem, "Depth Stencil");
			connect(depthStencilItem.partner, &QToolButton::clicked, this, &Wrap::newDepthStencilAttachment);
			if (p->depthStencilAttachment)
				p->depthStencilAttachment->appear(depthStencilItem.item, 1);

			dependenciesItem.partner = new QToolButton;
			setAddButton(dependenciesItem.partner);
			dependenciesItem.setup(tree, item.item, "Dependencies");
			connect(dependenciesItem.partner, &QToolButton::clicked, this, &Wrap::newDependency);
			for (auto &dependency : p->dependencies)
				dependency.appear(dependenciesItem.item);

			actionsItem.partner = new QToolButton;
			setAddButton(actionsItem.partner);
			actionsItem.setup(tree, item.item, "Action");
			connect(actionsItem.partner, &QToolButton::clicked, this, &Wrap::newAction);
			for (auto &action : p->actions)
				action.appear(actionsItem.item);
		}
		void up()
		{
			p->mark = tke::Element::eMarkUp;
			p->parent->maintain(0);
		}
		void down()
		{
			p->mark = tke::Element::eMarkDown;
			p->parent->maintain(0);
		}
		void remove()
		{
			p->mark = tke::Element::eMarkClear;
			p->parent->maintain(0);
		}

		void newColorAttachment()
		{
			auto attachment = p->addColorAttachment();
			std::stringstream name;
			name << p->colorAttachments.size();
			attachment->name = name.str();
			attachment->appear(colorsItem.item, 0);
		}

		void newDepthStencilAttachment()
		{
			if (p->depthStencilAttachment) return;
			auto attachment = p->addDepthStencilAttachment();
			attachment->name = "1";
			attachment->aspect = tke::AspectFlags::depth;
			attachment->appear(depthStencilItem.item, 1);
		}

		void newDependency()
		{
			auto dependency = p->addDependency();
			dependency->appear(dependenciesItem.item);
		}

		void newAction()
		{
			auto action = p->addAction();
			std::stringstream name;
			name << "Action";
			name << p->actions.size();
			action->name = name.str();
			action->appear(actionsItem.item);
		}
	}wrap;

	void appear(QTreeWidgetItem *parent)
	{
		wrap.appear(this, parent, &name);
	}
};

struct Renderer : tke::RendererAbstract<RenderPass>
{
	QListWidgetItem *listItem;
	struct Wrap : QObject
	{
		Renderer *p;
		tke::QTreeItemPair<QToolButton> passesItem;

		void newPass()
		{
			auto pass = p->addPass();
			std::stringstream name;
			name << "Render Pass";
			name << p->passes.size();
			pass->name = name.str();
			pass->appear(passesItem.item);
		}

		void reappearPassItem()
		{
			delete passesItem.item;
			passesItem.partner = new QToolButton;
			passesItem.setup(tree, reinterpret_cast<QTreeWidgetItem*>(tree), "passes");
			setAddButton(passesItem.partner);
			connect(passesItem.partner, &QToolButton::clicked, this, &Wrap::newPass);
			for (auto &pass : p->passes)
				pass.appear(passesItem.item);
		}

		void appear(Renderer *_p)
		{
			p = _p;
			for (auto r : p->b->reflectons)
			{
				if (r->name ==  "name")
					continue;
				tke::qAddTreeItem(tree, reinterpret_cast<QTreeWidgetItem*>(tree), p, r);
			}
			passesItem.item = nullptr;
			reappearPassItem();
		}
	}wrap;

	void appear()
	{
		wrap.appear(this);
	}
};
