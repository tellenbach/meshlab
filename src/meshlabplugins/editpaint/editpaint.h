/***************************************************************************
first version: 0.1 
autor: Gfrei Andreas 
date:  07/02/2007 
email: gfrei.andreas@gmx.net
****************************************************************************/

#ifndef EDITPAINT_H
#define EDITPAINT_H

#include <QObject>
#include <QStringList>
#include <QList>
#include <QDockWidget>
#include <QHash>
#include <QDialog>

#include <meshlab/meshmodel.h>
#include <meshlab/interfaces.h>
#include <vcg/math/matrix44.h>
#include "ui_painttoolbox.h"
#include "colorwid.h"

class EditPaintPlugin;
class PaintToolbox;
class Penn;
class ColorUndo;
class PaintWorker;
struct PaintData;
struct UndoItem;

int isIn(QPointF p0,QPointF p1,float dx,float dy,float raduis);


typedef enum {PEN, FILL, PICK, NONE, GRADIENT, SMOOTH, SELECT, POLY_SMOOTH} PaintThing;

class Penn {
public:
	float radius;
	//QPoint pos;
	int painttype;
	bool backface;
	bool invisible;
};

struct PaintData{
	int special;
	QPoint start;
	QPoint end;
	int opac;
	Color4b color;
	//double radius;
	Penn pen;
};

struct Vert_Data {
	CVertexO * v;
	float distance;
};

struct Vert_Data_2 {
	//CVertexO * v;
	float distance;
	Color4b color;
};

struct Vert_Data_3{
	//CVertexO * v;
	float distance;
	float pos[3];
	Color4b color;
};

class EditPaintPlugin : public QObject, public MeshEditInterface {
	Q_OBJECT
	Q_INTERFACES(MeshEditInterface)
	QList <QAction *> actionList;

public:
	EditPaintPlugin();
	
	virtual ~EditPaintPlugin();
	
	virtual const QString Info(QAction *);
	virtual const PluginInfo &Info();
	
	virtual void StartEdit(QAction * /*mode*/, MeshModel &/*m*/, GLArea * /*parent*/);
	virtual void EndEdit(QAction * /*mode*/, MeshModel &/*m*/, GLArea * /*parent*/);
	virtual void Decorate(QAction * /*mode*/, MeshModel &/*m*/, GLArea * /*parent*/);
	virtual void mousePressEvent    (QAction *, QMouseEvent *event, MeshModel &/*m*/, GLArea * );
	virtual void mouseMoveEvent     (QAction *,QMouseEvent *event, MeshModel &/*m*/, GLArea * );
	virtual void mouseReleaseEvent  (QAction *,QMouseEvent *event, MeshModel &/*m*/, GLArea * );
	//	  virtual void wheelEvent         (QAction *QWheelEvent*e, MeshModel &/*m*/, GLArea * );
	virtual QList<QAction *> actions() const ;
	QPoint start;
	QPoint cur;
	QPoint prev;
	bool isDragging;
	vector<CMeshO::FacePointer> LastSel;

private:
	//typedef enum {SMAdd, SMClear,SMSub} SelMode;
	//SelMode selMode;
	bool has_track; // to restore the trackball settings
	int pressed; // to check in decorate if it is the first call after a mouse down or mouse up
	bool first; // to check in decorate if it is the first call after a mouse down
	double mvmatrix[16]; //modelview
	double projmatrix[16]; //projection
	int viewport[4]; //viewport
	GLfloat *pixels; // the z-buffer
	int inverse_y; // gla->curSiz.height()-cur.y() TODO probably removable
	vector<CMeshO::FacePointer> tempSel; //to use when needed
	vector<CMeshO::FacePointer> curSel; //the faces i am painting on
	QHash<CVertexO *,Vert_Data_2> temporaneo; //the vertexes i am painting on
	Penn pen; //contains informations about the painting mode, color, type ...
	PaintToolbox *paintbox; //the widget with the painting stuff
	QDockWidget *paint_dock;
	Qt::MouseButton curr_mouse; // which mouse button is selected

	//PaintWorker * worker;
	GLArea* current_gla;
	QHash <GLArea *,ColorUndo *> color_undo;

	int paintType();
	void DrawXORRect(MeshModel &m,GLArea * gla, bool doubleDraw);
	void drawLine(GLArea * gla);
	void fillGradient(MeshModel &,GLArea * gla);
	bool getFaceAtMouse(MeshModel &,CMeshO::FacePointer &);
	bool getFacesAtMouse(MeshModel &,vector<CMeshO::FacePointer> &);
	bool getVertexAtMouse(MeshModel &,CMeshO::VertexPointer &);
	bool getVertexesAtMouse();
	void fillFrom(MeshModel &,CFaceO *);
	bool hasSelected(MeshModel &);

	void pushUndo(GLArea * gla);

	inline void updateMatrixes() {		
		glGetIntegerv(GL_VIEWPORT, viewport);
		glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
		glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);
		/*for (int lauf=0; lauf<4; lauf++) {
			qDebug() <<projmatrix[lauf*4]<<" "<<projmatrix[lauf*4+1]<<" "<<projmatrix[lauf*4+2]<<" "<<projmatrix[lauf*4+3]<<" ";
		}
		qDebug() <<"---- viewport: "<<viewport[0]<<" "<<viewport[1]<<" "<<viewport[2]<<" "<<viewport[3]<< "---------"<<endl;*/
	}

	inline int getNearest(QPointF center, QPointF *punti,int num) {
		int nearestInd=0;
		float dist=fabsf(center.x()-punti[0].x())*fabsf(center.x()-punti[0].x())+fabsf(center.y()-punti[0].y())*fabsf(center.y()-punti[0].y());
		for (int lauf=1; lauf<num; lauf++) {
			float temp=fabsf(center.x()-punti[lauf].x())*fabsf(center.x()-punti[lauf].x())+
				fabsf(center.y()-punti[lauf].y())*fabsf(center.y()-punti[lauf].y());
			if (temp<dist) {
				nearestInd=lauf;
				dist=temp;
			}
		}
		return nearestInd;
	}

public slots:
	void undo(int value);
	//void redo();
};

struct UndoItem {
	CVertexO * vertex;
	Color4b original;
	//Color4b new_color;
};

class ColorUndo {
private:
	vector<vector<UndoItem> *> undos;
	vector<vector<UndoItem> *> redos;
	vector<UndoItem> * temp_vector; 
public:
	void pushUndo() { 
		if (undos.size()==15) removeUndo();
		if (temp_vector->size()==0) return;
		for (int lauf=0; lauf<redos.size(); lauf++) { redos[lauf]->clear(); delete redos[lauf];}
		redos.clear();
		undos.push_back(temp_vector);
		temp_vector=new vector<UndoItem>();
	}
	void undo();
	void redo();
	void removeUndo() { 
		if (undos.size()==0) return; 
		undos[0]->clear();
		delete undos[0];
		undos.erase(undos.begin());
	}
	bool hasUndo() { return undos.size()!=0; }
	bool hasRedo() { return redos.size()!=0; }
	inline void addItem(UndoItem u) { temp_vector->push_back(u); }
	ColorUndo() { temp_vector=new vector<UndoItem>();}
};

class PaintWorker : public QThread{
Q_OBJECT
private:
	MeshModel* mesh;
	GLArea *gla;
	QVector <PaintData> dati;
	QMutex mutex;
	QWaitCondition condition;
	bool nothingTodo;

	double mvmatrix[16];
	double projmatrix[16];
	int viewport[4];

	vector<CMeshO::FacePointer> curSel;
	QHash<CVertexO *,Color4b> temporaneo;
	GLfloat *pixels;

	void run();
public:
	PaintWorker();
	//inline bool waitsForData() { return nothingTodo; }
	inline void waitTillPause() { mutex.lock(); if (nothingTodo) {mutex.unlock(); return;} condition.wait(&mutex); mutex.unlock(); return; }
	inline void setModelArea(MeshModel * mo, GLArea * a) {  mesh=mo; gla=a; }
	inline void clear(GLfloat * pi) { 
		curSel.clear(); temporaneo.clear(); pixels=pi; 
		glGetIntegerv(GL_VIEWPORT, viewport);
		glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
		glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);
	}
//public slots:
	void addData(PaintData data) { mutex.lock(); dati.push_back(data); condition.wakeAll(); mutex.unlock(); }
};

class PaintToolbox : public QWidget {
Q_OBJECT
public:
	PaintToolbox ( /*const QString & title,*/ QWidget * parent = 0, Qt::WindowFlags flags = 0 );
	Color4b getColor(Qt::MouseButton);
	void setColor(Color4b,Qt::MouseButton);
	void setColor(int,int,int,Qt::MouseButton mouse);
	double getRadius() { if (ui.tabWidget->currentIndex()==1) return ui.pen_radius_2->value(); return ui.pen_radius->value(); }
	int paintType() { if (ui.tabWidget->currentIndex()==1) return ui.pen_type_2->currentIndex()+1; return ui.pen_type->currentIndex()+1; }
	int searchMode() { return ui.search_mode->currentIndex()+1; }
	int getOpacity() { return ui.deck_slider->value(); }
	int getSmoothPercentual() { if (ui.tabWidget->currentIndex()==1) return ui.percentual_slider_2->value(); return ui.percentual_slider->value(); }
	int getDecreasePercentual() { if (ui.tabWidget->currentIndex()==1) return ui.decrease_slider_2->value(); return ui.decrease_slider->value(); }
	int paintUtensil() { if (ui.tabWidget->currentIndex()<2) return paint_utensil[ui.tabWidget->currentIndex()]; else return NONE; }
	bool getPaintBackface() { if (ui.tabWidget->currentIndex()==1) return ui.backface_culling_2->checkState()!=Qt::Unchecked; 
					return ui.backface_culling->checkState()!=Qt::Unchecked; }
	bool getPaintInvisible() { if (ui.tabWidget->currentIndex()==1) return ui.invisible_painting_2->checkState()!=Qt::Unchecked; 
					return ui.invisible_painting->checkState()!=Qt::Unchecked; }

	inline int getGradientType() { return ui.gradient_type->currentIndex(); }
	inline int getGradientForm() { return ui.gradient_form->currentIndex(); }

	inline int getPickMode() { return ui.pick_mode->currentIndex(); }
	void setUndo(bool value) { ui.undo_button->setEnabled(value); }
	void setRedo(bool value) { ui.redo_button->setEnabled(value); }
private:
	int paint_utensil[2];
	Ui::PaintToolbox ui;
private slots:
	void on_pen_type_currentIndexChanged(QString value);
	void on_pen_radius_valueChanged(double value);
	void on_switch_me_clicked();
	void on_set_bw_clicked();
	void on_deck_slider_valueChanged(int value);
	void on_deck_box_valueChanged(int value);
	void on_pen_button_clicked();
	void on_fill_button_clicked();
	void on_pick_button_clicked();
	//void on_advanced_button_clicked();
	void on_backface_culling_stateChanged(int value);
	void on_invisible_painting_stateChanged(int value);
	void on_undo_button_clicked();
	void on_redo_button_clicked();
	void on_gradient_button_clicked();
	void on_smooth_button_clicked();
	void on_percentual_slider_valueChanged(int value);
	void on_percentual_box_valueChanged(int value);
	void on_tabWidget_currentChanged ( int index );
	void on_select_button_clicked();
	void on_poly_smooth_button_clicked();

	void on_decrease_slider_valueChanged(int value);
	void on_decrease_box_valueChanged(int value);

	void on_percentual_slider_2_valueChanged(int value);
	void on_percentual_box_2_valueChanged(int value);
	void on_decrease_slider_2_valueChanged(int value);
	void on_decrease_box_2_valueChanged(int value);
	void on_pen_type_2_currentIndexChanged(QString value);
	void on_pen_radius_2_valueChanged(double value);
signals:
	void undo_redo(int value);
	//void redo();
};

#endif
