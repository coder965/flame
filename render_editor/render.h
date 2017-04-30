void setAddButton(QToolButton *b)
{
	b->setMaximumWidth(21);
	b->setMaximumHeight(21);
	b->setIcon(QIcon(":png/add.png"));
}

void setRemoveButton(QToolButton *b)
{
	b->setMaximumWidth(21);
	b->setMaximumHeight(21);
	b->setIcon(QIcon(":png/delete.png"));
}

void setUpButton(QToolButton *b)
{
	b->setMaximumWidth(21);
	b->setMaximumHeight(21);
	b->setIcon(QIcon(":png/up.png"));
}

void setDownButton(QToolButton *b)
{
	b->setMaximumWidth(21);
	b->setMaximumHeight(21);
	b->setIcon(QIcon(":png/down.png"));
}

namespace tk
{
	namespace Engine
	{
		void addItem(QTreeWidget *tree, QTreeWidgetItem *parent, void *p, Reflection *r)
		{
			if (r->what == Reflection::eVariable)
			{
				auto v = (VariableReflection*)r;
				if (v->type() == typeid(int))
				{
					QTreeItemPair<QIntEdit> item;
					item.partner = new QIntEdit(v->ptr<int>(p), 0, 10000);
					item.setup(tree, parent, v->name.c_str());
				}
				else if (v->type() == typeid(float))
				{
					QTreeItemPair<QFloatEdit> item;
					item.partner = new QFloatEdit(v->ptr<float>(p));
					item.setup(tree, parent, v->name.c_str());
				}
				else if (v->type() == typeid(bool))
				{
					QTreeItemPair<QBoolCheck> item;
					item.partner = new QBoolCheck(v->ptr<bool>(p));
					item.setup(tree, parent, v->name.c_str());
				}
				else if (v->type() == typeid(std::string))
				{
					QTreeItemPair<QStringCombo> item;
					item.partner = new QStringCombo(v->ptr<std::string>(p));
					item.setup(tree, parent, v->name.c_str());
				}
			}
			else if (r->what == Reflection::eEnum)
			{
				auto e = (EnumReflection*)r;
				auto v = e->ptr(p);
				QTreeItemPair<QIntDropCombo> item;
				item.partner = new QIntDropCombo(v);
				for (auto &i : e->pEnum->items)
					item.partner->addItem(i.first.c_str());
				item.partner->setCurrentIndex(*v);
				item.setup(tree, parent, e->name.c_str());
			}
		}

		struct RenderPassExtraData;
		struct RendererExtraData;

		struct DrawcallExtraData : ExtType, QObject
		{
			Drawcall *pDrawcall = nullptr;

			QTreeItemPair<QToolButton> item;

			~DrawcallExtraData()
			{
				delete item.item;
			}

			void destroyThis()
			{
				pDrawcall->parent->removeDrawcall(pDrawcall);
			}

			void appear(QTreeWidgetItem *parent)
			{
				item.partner = new QToolButton;
				setRemoveButton(item.partner);
				item.setup(tree, parent, pDrawcall->name.c_str(), &pDrawcall->name);
				connect(item.partner, &QToolButton::clicked, this, &DrawcallExtraData::destroyThis);

				for (auto r : pDrawcall->b->reflectons)
				{
					if (r->what == Reflection::eVariable && r->toVar()->name == "name")
						continue;
					addItem(tree, item.item, pDrawcall, r);
				}
			}
		};

		struct DrawActionExtraData : ExtType, QObject
		{
			DrawAction *pAction = nullptr;

			QTreeItemPair<QToolButton> item;
			QTreeItemPair<QIntDropCombo> typeItem;
			QTreeItemPair<QToolButton> drawcallsItem;

			~DrawActionExtraData()
			{
				delete item.item;
			}

			void destroyThis()
			{
				pAction->parent->removeAction(pAction);
			}

			void cleanUp()
			{
				for (auto dc : pAction->m_drawcalls)
					dc->ext->item.item = nullptr;
			}

			void newDrawcall()
			{
				auto drawcall = std::make_shared<Drawcall>();
				std::stringstream name;
				name << pAction->m_drawcalls.size() + 1;
				drawcall->name = name.str();
				drawcall->ext = new DrawcallExtraData;
				drawcall->ext->pDrawcall = drawcall.get();
				pAction->addDrawcall(drawcall);
				drawcall->ext->appear(drawcallsItem.item);
			}

			void appear(QTreeWidgetItem *parent)
			{
				item.partner = new QToolButton;
				setRemoveButton(item.partner);
				item.setup(tree, parent, pAction->name.c_str(), &pAction->name);
				connect(item.partner, &QToolButton::clicked, this, &DrawActionExtraData::destroyThis);

				for (auto r : pAction->b->reflectons)
				{
					if (r->what == Reflection::eVariable && r->toVar()->name == "name")
						continue;
					addItem(tree, item.item, pAction, r);
				}

				drawcallsItem.partner = new QToolButton;
				setAddButton(drawcallsItem.partner);
				drawcallsItem.setup(tree, item.item, "Drawcalls");
				connect(drawcallsItem.partner, &QToolButton::clicked, this, &DrawActionExtraData::newDrawcall);

				for (auto drawcall : pAction->m_drawcalls)
					drawcall->ext->appear(drawcallsItem.item);
			}
		};

		struct AttachmentExtraData : ExtType, QObject
		{
			Attachment *pAttachment = nullptr;

			int type;

			QTreeItemPair<QToolButton> item;

			~AttachmentExtraData()
			{
				delete item.item;
			}

			void destroyThis()
			{
				if (type == 0)
					pAttachment->parent->removeColorAttachment(pAttachment);
				else if (type == 1)
					pAttachment->parent->removeDepthStencilAttachment();
			}

			void appear(QTreeWidgetItem *parent, int _type)
			{
				type = _type;

				item.partner = new QToolButton;
				setRemoveButton(item.partner);
				item.setup(tree, parent, pAttachment->name.c_str(), &pAttachment->name);
				connect(item.partner, &QToolButton::clicked, this, &AttachmentExtraData::destroyThis);

				for (auto r : pAttachment->b->reflectons)
				{
					if (r->what == Reflection::eVariable && r->toVar()->name == "name")
						continue;
					addItem(tree, item.item, pAttachment, r);
				}
			}
		};

		struct DependencyExtraData : ExtType, QObject
		{
			Dependency *pDependency = nullptr;

			QTreeWidgetItem *item;
			QComboBox *combo;
			QToolButton *deleteButton;

			~DependencyExtraData()
			{
				delete item;
			}

			void destroyThis()
			{
				pDependency->parent->removeDependency(pDependency);
			}

			void setDependency(int index)
			{
				if (index == -1)
				{
					pDependency->target = nullptr;
					return;
				}
				pDependency->target = pDependency->parent->parent->passes[index].get();
			}

			void setComboIndex();

			void appear(QTreeWidgetItem *parent);

		};

		struct RenderPassExtraData : ExtType, QObject
		{
			RenderPass *pRenderPass = nullptr;

			int index;

			QTreeItemPair<QGroupBox> item;
			QTreeWidgetItem *attachmentsItem = nullptr;
			QTreeItemPair<QToolButton> colorsItem;
			QTreeItemPair<QToolButton> depthStencilItem;
			QTreeItemPair<QToolButton> dependenciesItem;
			QTreeItemPair<QToolButton> actionsItem;

			~RenderPassExtraData()
			{
				cleanUp();
				delete item.item;
			}

			void cleanUp()
			{
				for (auto action : pRenderPass->actions)
				{
					action->ext->cleanUp();
					action->ext->item.item = nullptr;
				}
				if (pRenderPass->depthStencilAttachment.get())
					pRenderPass->depthStencilAttachment->ext->item.item = nullptr;
				for (auto a : pRenderPass->colorAttachments)
					a->ext->item.item = nullptr;
				for (auto d : pRenderPass->dependencies)
					d->ext->item = nullptr;
			}

			void destroyThis()
			{
				auto parent = pRenderPass->parent;
				pRenderPass->parent->removePass(pRenderPass);
				for (auto p : parent->passes)
				{
					for (auto it = p->dependencies.begin(); it != p->dependencies.end(); )
					{
						(*it)->ext->setComboIndex();
						if ((*it)->target == nullptr)
						{
							it = p->dependencies.erase(it);
						}
						else
						{
							p->ext->setDependencyCombo((*it)->ext->combo);
							it++;
						}
					}
				}
			}

			void upThis();

			void downThis();

			void newColorAttachment()
			{
				auto attachment = std::make_shared<Attachment>();
				std::stringstream name;
				name << pRenderPass->colorAttachments.size() + 1;
				attachment->name = name.str();
				attachment->ext = new AttachmentExtraData;
				attachment->ext->pAttachment = attachment.get();
				pRenderPass->addColorAttachment(attachment);
				attachment->ext->appear(colorsItem.item, 0);
			}

			void newDepthStencilAttachment()
			{
				if (pRenderPass->depthStencilAttachment) return;
				auto attachment = std::make_shared<Attachment>();
				attachment->name = "1";
				attachment->aspect = AspectFlags::depth;
				attachment->ext = new AttachmentExtraData;
				attachment->ext->pAttachment = attachment.get();
				pRenderPass->addDepthStencilAttachment(attachment);
				attachment->ext->appear(depthStencilItem.item, 1);
			}

			void newDependency()
			{
				auto dependency = std::make_shared<Dependency>();
				dependency->ext = new DependencyExtraData;
				dependency->ext->pDependency = dependency.get();
				pRenderPass->addDependency(dependency);
				dependency->ext->appear(dependenciesItem.item);
			}

			void setDependencyCombo(QComboBox *combo)
			{
				combo->clear();
				for (auto pass : pRenderPass->parent->passes)
					combo->addItem(pass->name.c_str());
			}

			void newAction()
			{
				auto action = std::make_shared<DrawAction>();
				std::stringstream name;
				name << "Action";
				name << pRenderPass->actions.size() + 1;
				action->name = name.str();
				action->ext = new DrawActionExtraData;
				action->ext->pAction = action.get();
				pRenderPass->addAction(action);
				action->ext->appear(actionsItem.item);
			}

			void setSecondaryCmd(const QString &str)
			{
				pRenderPass->secondary_cmd_name = str.toUtf8().data();
			}

			void appear(QTreeWidgetItem *parent)
			{
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
				item.setup(tree, parent, pRenderPass->name.c_str(), &pRenderPass->name);
				connect(removeButton, &QToolButton::clicked, this, &RenderPassExtraData::destroyThis);
				connect(upButton, &QToolButton::clicked, this, &RenderPassExtraData::upThis);
				connect(downButton, &QToolButton::clicked, this, &RenderPassExtraData::downThis);

				for (auto r : pRenderPass->b->reflectons)
				{
					if (r->what == Reflection::eVariable && r->toVar()->name == "name")
						continue;
					addItem(tree, item.item, pRenderPass, r);
				}

				attachmentsItem = new QTreeWidgetItem(item.item, QStringList("Attachments"));

				colorsItem.partner = new QToolButton;
				setAddButton(colorsItem.partner);
				colorsItem.setup(tree, attachmentsItem, "Color");
				connect(colorsItem.partner, &QToolButton::clicked, this, &RenderPassExtraData::newColorAttachment);
				for (auto color : pRenderPass->colorAttachments)
					color->ext->appear(colorsItem.item, 0);

				depthStencilItem.partner = new QToolButton;
				setAddButton(depthStencilItem.partner);
				depthStencilItem.setup(tree, attachmentsItem, "Depth Stencil");
				connect(depthStencilItem.partner, &QToolButton::clicked, this, &RenderPassExtraData::newDepthStencilAttachment);
				if (pRenderPass->depthStencilAttachment)
					pRenderPass->depthStencilAttachment->ext->appear(depthStencilItem.item, 1);

				dependenciesItem.partner = new QToolButton;
				setAddButton(dependenciesItem.partner);
				dependenciesItem.setup(tree, item.item, "Dependencies");
				connect(dependenciesItem.partner, &QToolButton::clicked, this, &RenderPassExtraData::newDependency);
				for (auto dependency : pRenderPass->dependencies)
					dependency->ext->appear(dependenciesItem.item);

				actionsItem.partner = new QToolButton;
				setAddButton(actionsItem.partner);
				actionsItem.setup(tree, item.item, "Action");
				connect(actionsItem.partner, &QToolButton::clicked, this, &RenderPassExtraData::newAction);
				for (auto action : pRenderPass->actions)
					action->ext->appear(actionsItem.item);
			}
		};

		struct RendererExtraData : ExtType, QObject
		{
			Renderer *pRenderer = nullptr;

			QTreeItemPair<QToolButton> passesItem;

			QListWidgetItem *listItem = nullptr;

			void cleanUp()
			{
				for (auto p : pRenderer->passes)
				{
					p->ext->cleanUp();
					p->ext->item.item = nullptr;
				}
			}

			void newPass()
			{
				auto pass = std::make_shared<RenderPass>();

				std::stringstream name;
				name << "Render Pass";
				name << pRenderer->passes.size() + 1;
				pass->name = name.str();
				pass->ext = new RenderPassExtraData;
				pass->ext->pRenderPass = pass.get();
				pRenderer->addPass(pass);
				for (auto p : pRenderer->passes)
				{
					if (p == pass) continue;
					for (auto d : p->dependencies)
					{
						p->ext->setDependencyCombo(d->ext->combo);
						d->ext->setComboIndex();
					}
				}
				pass->ext->appear(passesItem.item);
			}

			void reappearPassItem()
			{
				delete passesItem.item;
				passesItem.partner = new QToolButton;
				passesItem.setup(tree, reinterpret_cast<QTreeWidgetItem*>(tree), "passes");
				setAddButton(passesItem.partner);
				connect(passesItem.partner, &QToolButton::clicked, this, &RendererExtraData::newPass);
				for (auto pass : pRenderer->passes)
					pass->ext->appear(passesItem.item);
			}

			void appear()
			{
				for (auto r : pRenderer->b->reflectons)
				{
					if (r->what == Reflection::eVariable && (r->toVar()->name == "name" || r->toVar()->name == "filename"))
						continue;
					addItem(tree, reinterpret_cast<QTreeWidgetItem*>(tree), pRenderer, r);
				}

				reappearPassItem();
			}
		};


		void RenderPassExtraData::upThis()
		{
			if (pRenderPass->parent->upPass(pRenderPass) != -1)
				pRenderPass->parent->ext->reappearPassItem();
		}

		void RenderPassExtraData::downThis()
		{
			if (pRenderPass->parent->downPass(pRenderPass) != -1)
				pRenderPass->parent->ext->reappearPassItem();
		}

		void DependencyExtraData::setComboIndex()
		{
			auto renderer = pDependency->parent->parent;
			for (auto i = 0; i < renderer->passes.size(); i++)
			{
				if (renderer->passes[i].get() == pDependency->target)
				{
					combo->setCurrentIndex(i);
					if (i == -1)
						pDependency->target = nullptr;
				}
			}
		}

		void DependencyExtraData::appear(QTreeWidgetItem *parent)
		{
			combo = new QComboBox;
			pDependency->parent->ext->setDependencyCombo(combo);
			setComboIndex();
			connect(combo, (void(QComboBox::*)(int))&QComboBox::currentIndexChanged, this, &DependencyExtraData::setDependency);
			item = new QTreeWidgetItem(parent);
			deleteButton = new QToolButton;
			setRemoveButton(deleteButton);
			tree->setItemWidget(item, 0, combo);
			tree->setItemWidget(item, 1, deleteButton);
			connect(deleteButton, &QToolButton::clicked, this, &DependencyExtraData::destroyThis);
		}

		void setupRenderer(Renderer *r)
		{
			r->ext = new RendererExtraData;
			r->ext->pRenderer = r;
			for (auto pass : r->passes)
			{
				pass->ext = new RenderPassExtraData;
				pass->ext->pRenderPass = pass.get();
				for (auto color : pass->colorAttachments)
				{
					color->ext = new AttachmentExtraData;
					color->ext->pAttachment = color.get();
				}
				if (pass->depthStencilAttachment)
				{
					pass->depthStencilAttachment->ext = new AttachmentExtraData;
					pass->depthStencilAttachment->ext->pAttachment = pass->depthStencilAttachment.get();
				}
				for (auto dependency : pass->dependencies)
				{
					dependency->ext = new DependencyExtraData;
					dependency->ext->pDependency = dependency.get();
				}
				for (auto action : pass->actions)
				{
					action->ext = new DrawActionExtraData;
					action->ext->pAction = action.get();
					for (auto drawcall : action->m_drawcalls)
					{
						drawcall->ext = new DrawcallExtraData;
						drawcall->ext->pDrawcall = drawcall.get();
					}
				}
			}
		}
	}
}
