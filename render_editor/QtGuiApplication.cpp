void QtGuiApplication::on_addButton_clicked()
{
	auto list = QFileDialog::getOpenFileNames(this, "", rendererPath.c_str());
	if (list.size() == 0) return;

	for (auto i = 0; i < list.size(); i++)
	{
		auto renderer = new Renderer;
		renderer->filename = list[i].toUtf8().data();
		renderer->loadXML();
		renderer->listItem = new QListWidgetItem;
		std::string title = renderer->filename;
		if (title.compare(0, rendererPath.size(), rendererPath) == 0)
			title = title.c_str() + rendererPath.size();
		renderer->listItem->setText(title.c_str());
		ui.listWidget->addItem(renderer->listItem);
		renderers.push_back(renderer);
	}

	saveDataXml();
}

void QtGuiApplication::on_removeButton_clicked() 
{
	if (!currentRenderer) return;

	for (auto it = renderers.begin(); it != renderers.end(); it++)
	{
		if (*it == currentRenderer)
		{
			renderers.erase(it);
			break;
		}
	}

	delete currentRenderer->listItem;
	delete currentRenderer;
	currentRenderer = nullptr;

	saveDataXml();
}

void QtGuiApplication::on_listWidget_currentItemChanged(QListWidgetItem *_curr, QListWidgetItem *_prev)
{
	tree->clear();
	for (auto r : renderers)
	{
		if (r->listItem == _curr)
		{
			currentRenderer = r;
			currentRenderer->appear();
			return;
		}
	}
}

void QtGuiApplication::on_treeWidget_itemChanged(QTreeWidgetItem *item, int column)
{
	auto str = (std::string*)item->data(0, Qt::UserRole).value<unsigned int>();
	*str = item->text(0).toUtf8().data();
}

void QtGuiApplication::on_saveButton_clicked()
{
	if (currentRenderer)
		currentRenderer->saveXML();
}