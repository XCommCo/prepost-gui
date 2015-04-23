#include "cgnsfileinputconditioncontainerset.h"
#include "cgnsfileinputconditiondialog.h"

#include <misc/xmlsupport.h>
#include <misc/errormessage.h>

#include <QDomNode>
#include <QDomNodeList>
#include <QDomElement>
#include <QMessageBox>

CgnsFileInputConditionContainerSet::CgnsFileInputConditionContainerSet(QWidget* widget)
{
	m_parentWidget = widget;
}

void CgnsFileInputConditionContainerSet::clear(){
	m_integers.clear();
	m_reals.clear();
	m_strings.clear();
	m_functionals.clear();
	m_containers.clear();
}
void CgnsFileInputConditionContainerSet::setup(const QDomNode& condNode, bool forBC){
	// setup containers;
	clear();
	if (forBC){
		setupCustom(condNode);
	} else {
		// multiple pages exists.
		QDomNodeList pages = condNode.childNodes();
		for (unsigned int i = 0; i < pages.length(); ++i){
			QDomNode page = pages.item(i);
			// access content/items/
			setupCustom(page);
		}
	}
}

void CgnsFileInputConditionContainerSet::setBCProperty(const QString& bcname, int bcindex)
{
	QMap<QString, CgnsFileInputConditionContainer*>::iterator it;
	for (it = m_containers.begin(); it != m_containers.end(); ++it){
		it.value()->setBCProperty(bcname, bcindex);
	}
}

void CgnsFileInputConditionContainerSet::setComplexProperty(const QString& compname, int compindex)
{
	QMap<QString, CgnsFileInputConditionContainer*>::iterator it;
	for (it = m_containers.begin(); it != m_containers.end(); ++it){
		it.value()->setComplexProperty(compname, compindex);
	}
}

void CgnsFileInputConditionContainerSet::setupSimple(const QDomNode& contNode)
{
	QDomNode itemsNode = iRIC::getChildNode(contNode, "Items");
	if (itemsNode.isNull()){return;}
	// Now, I've got the itemsNode!
	QDomNodeList items = itemsNode.childNodes();
	for (unsigned int j = 0; j < items.length(); ++j){
		QDomNode itemNode = items.item(j);
		setupContaner(itemNode);
	}
}

void CgnsFileInputConditionContainerSet::setupCustom(const QDomNode& contNode)
{
	setupCustomRec(contNode);
}

void CgnsFileInputConditionContainerSet::setupCustomRec(const QDomNode& node){
	QDomNodeList children = node.childNodes();
	for (unsigned int i = 0; i < children.length(); ++i){
		QDomNode c = children.item(i);
		if (c.nodeType() == QDomNode::ElementNode){
			if (c.nodeName() == "Item"){
				// build item.
				setupContaner(c);
			} else {
				setupCustomRec(c);
			}
		}
	}
}
void CgnsFileInputConditionContainerSet::setupContaner(const QDomNode& itemNode){
	QString parameterName;
	try {
		/// get the name;
		QDomElement itemElem = itemNode.toElement();
		parameterName = itemElem.attribute("name");
		// get the definition node;
		QDomNode defNode = iRIC::getChildNode(itemNode, "Definition");
		if (defNode.isNull()){
			// this item doesn't contain "Definition" Node!".
			throw (ErrorMessage(tr("Definition node is not stored")));
		}
		QDomElement defElem = defNode.toElement();
		QString type = defElem.attribute("conditionType");
		// setup container depending on the type
		if (type == "functional"){
			m_functionals.insert(parameterName, CgnsFileInputConditionContainerFunctional(parameterName, defNode));
			m_containers.insert(parameterName, &(m_functionals[parameterName]));
			connect(&(m_functionals[parameterName]), SIGNAL(valueChanged()), this, SIGNAL(modified()));
		} else if (type == "constant" || type == ""){
			QString valuetype = defElem.attribute("valueType");
			if (valuetype == "integer"){
				m_integers.insert(parameterName, CgnsFileInputConditionContainerInteger(parameterName, defNode));
				m_containers.insert(parameterName, &(m_integers[parameterName]));
				connect(&(m_integers[parameterName]), SIGNAL(valueChanged()), this, SIGNAL(modified()));
			} else if (valuetype == "real"){
				m_reals.insert(parameterName, CgnsFileInputConditionContainerReal(parameterName, defNode));
				m_containers.insert(parameterName, &(m_reals[parameterName]));
				connect(&(m_reals[parameterName]), SIGNAL(valueChanged()), this, SIGNAL(modified()));
			} else if (valuetype == "string" || valuetype == "filename" || valuetype == "filename_all" || valuetype == "foldername"){
				m_strings.insert(parameterName, CgnsFileInputConditionContainerString(parameterName, defNode));
				m_containers.insert(parameterName, &(m_strings[parameterName]));
				connect(&(m_strings[parameterName]), SIGNAL(valueChanged()), this, SIGNAL(modified()));
			} else if (valuetype == "functional"){
				m_functionals.insert(parameterName, CgnsFileInputConditionContainerFunctional(parameterName, defNode));
				m_containers.insert(parameterName, &(m_functionals[parameterName]));
				connect(&(m_functionals[parameterName]), SIGNAL(valueChanged()), this, SIGNAL(modified()));
			} else {
				throw (ErrorMessage(tr("Wrong valueType \"%1\" is set.").arg(valuetype)));
			}
		} else {
			throw (ErrorMessage(tr("Wrong conditionType \"%1\"is set.").arg(type)));
		}
	} catch (ErrorMessage& e){
		QString msg(tr("Error occured while loading solver definition file.\n%1: %2"));
		QMessageBox::critical(parentWidget(), tr("Error"), msg.arg(parameterName).arg(e));
		throw;
	}
}

void CgnsFileInputConditionContainerSet::setDefaultValues(){
	QMap<QString, CgnsFileInputConditionContainer*>::iterator it;
	for (it = m_containers.begin(); it != m_containers.end(); ++it){
		(*it)->clear();
	}
}

int CgnsFileInputConditionContainerSet::load(){
	// @todo no error checking (for example, wrong value, lacking nodes..) are implemented.
	QMap<QString, CgnsFileInputConditionContainer*>::iterator it;
	for (it = m_containers.begin(); it != m_containers.end(); ++it){
		(*it)->load();
	}
	return 0;
}
int CgnsFileInputConditionContainerSet::save(){
	QMap<QString, CgnsFileInputConditionContainer*>::iterator it;
	for (it = m_containers.begin(); it != m_containers.end(); ++it){
		int ret = (*it)->save();
		if (ret != 0){return ret;}
	}
	return 0;
}

void CgnsFileInputConditionContainerSet::reset()
{
	QMap<QString, CgnsFileInputConditionContainer*>::iterator it;
	for (it = m_containers.begin(); it != m_containers.end(); ++it){
		(*it)->clear();
	}
}

CgnsFileInputConditionContainerSet* CgnsFileInputConditionContainerSet::clone() const
{
	CgnsFileInputConditionContainerSet* ret = new CgnsFileInputConditionContainerSet(m_parentWidget);
	ret->m_integers = m_integers;
	ret->m_reals = m_reals;
	ret->m_strings = m_strings;
	ret->m_functionals = m_functionals;

	QMap<QString, CgnsFileInputConditionContainerInteger>::iterator it_int;
	for (it_int = ret->m_integers.begin(); it_int != ret->m_integers.end(); ++it_int){
		ret->m_containers.insert(it_int.key(), &(it_int.value()));
	}
	QMap<QString, CgnsFileInputConditionContainerReal>::iterator it_real;
	for (it_real = ret->m_reals.begin(); it_real != ret->m_reals.end(); ++it_real){
		ret->m_containers.insert(it_real.key(), &(it_real.value()));
	}
	QMap<QString, CgnsFileInputConditionContainerString>::iterator it_str;
	for (it_str = ret->m_strings.begin(); it_str != ret->m_strings.end(); ++it_str){
		ret->m_containers.insert(it_str.key(), &(it_str.value()));
	}
	QMap<QString, CgnsFileInputConditionContainerFunctional>::iterator it_func;
	for (it_func = ret->m_functionals.begin(); it_func != ret->m_functionals.end(); ++it_func){
		ret->m_containers.insert(it_func.key(), &(it_func.value()));
	}

	return ret;
}

void CgnsFileInputConditionContainerSet::copyValues(const CgnsFileInputConditionContainerSet* set)
{
	QMap<QString, CgnsFileInputConditionContainerInteger>::iterator it_int;
	for (it_int = m_integers.begin(); it_int != m_integers.end(); ++it_int){
		it_int.value() = set->m_integers.value(it_int.key());
	}
	QMap<QString, CgnsFileInputConditionContainerReal>::iterator it_real;
	for (it_real = m_reals.begin(); it_real != m_reals.end(); ++it_real){
		it_real.value() = set->m_reals.value(it_real.key());
	}
	QMap<QString, CgnsFileInputConditionContainerString>::iterator it_str;
	for (it_str = m_strings.begin(); it_str != m_strings.end(); ++it_str){
		it_str.value() = set->m_strings.value(it_str.key());
	}
	QMap<QString, CgnsFileInputConditionContainerFunctional>::iterator it_func;
	for (it_func = m_functionals.begin(); it_func != m_functionals.end(); ++it_func){
		it_func.value() = set->m_functionals.value(it_func.key());
	}
}