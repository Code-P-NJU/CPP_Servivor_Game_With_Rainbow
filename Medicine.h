#pragma once
#include "config.h"
#include <QPixmap>
#include <QRect>
class Medicine
{
public:
	QPixmap img;
	int healing=50;
	QRect rect;
	Medicine(QPoint p) :rect(p.x(), p.y(), 30, 30)
	{
		img.load(MEDICINE_IMG);
	}
};