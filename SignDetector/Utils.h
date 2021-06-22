#pragma once
#include "stdafx.h"
struct m_settings
{
	int NumberGate = 1;
	int NumberQueues = 1;
	int MaxNumberQueues = 5;
	int MinPeopleQueues = 5;
	int CamFPS = 30;
	int NotMovingShiftPIX = 10;
	int64 MinutesDelay = 1;
	bool FindRibbonMaze = false;
	int minPeopleToFindRibbons = 2;
	bool ShowImageDebug = false;
	//
	int MinBoarderLight = 30;
	int MaxBoarderLight = 300;
	int MinSquareRibbon = 50;
	int MaxSquareRibbon = 500;
	int MinLengthRibbon = 10;
	int MinSummAngle = 1;
	int LimitRibbons = 2;
	int CntRightStructRibbons = 200;
	//
	int MinPartHeightFrame = 5;
	int ElongatedRibbonRatioLengthToHeight = 100;
	int AnglSeparateHorizontalAndVerticalLines = 5;
};
void FindRibbons(cv::Mat img)
{

	//находим ограждения в виде лент на видео...для Кольцово
	cv::Mat image, gray, probabilistic_hough;
	cv::Mat mat_color[3];
	cv::Mat K;
	cv::Mat hsl;
	cv::Mat I, dst, hsv;

	img.copyTo(image);
	cv::Mat xmat = (cv::Mat_<char>(3, 3) << 0, -1, 0,
		0, 5, 0,
		0, -1, 0);
	auto kern = cv::Mat(cv::Size(3, 3), CV_8UC1);
	xmat.copyTo(kern);
	//auto t = (double)getTickCount();
	filter2D(image, K, image.depth(), kern);
	cvtColor(K, hsv, cv::COLOR_BGR2HSV);

	cv::Mat1b mask1, mask2;
	inRange(hsv, cv::Scalar(0, 70, 50), cv::Scalar(10, 255, 255), mask1);
	inRange(hsv, cv::Scalar(170, 70, 50), cv::Scalar(200, 255, 255), mask2);

	cv::Mat1b mask = mask1 | mask2;
	//K = K & Mat(mask);
	//imshow("Mask", mask);
	//dilate(K, K, kern, cv::Point(-1, -1), 3);


	for (int i = 2; i < mask.rows - 2; ++i)
		for (int j = 2; j < mask.cols - 2; ++j)
		{
			if (((mask.at<uchar>(i - 1, j) > m_settings.MinBoarderLight && mask.at<uchar>(i + 1, j) > m_settings.MinBoarderLight) ||
				(mask.at<uchar>(i, j - 1) > m_settings.MinBoarderLight && mask.at<uchar>(i, j + 1) > m_settings.MinBoarderLight) ||
				(mask.at<uchar>(i, j - 2) > m_settings.MinBoarderLight && mask.at<uchar>(i, j + 2) > m_settings.MinBoarderLight && mask.at<uchar>(i, j - 1) == 0 && mask.at<uchar>(i, j + 1) == 0) ||
				(mask.at<uchar>(i - 2, j) > m_settings.MinBoarderLight && mask.at<uchar>(i + 2, j) > m_settings.MinBoarderLight && mask.at<uchar>(i - 1, j) == 0 && mask.at<uchar>(i + 1, j) == 0))
				&& mask.at<uchar>(i, j) == 0)
			{
				//((_I.at<Vec3b>(i - 1, j)[0] - _I.at<Vec3b>(i, j)[0]) > delta ||
					//(_I.at<Vec3b>(i + 1, j)[0] - _I.at<Vec3b>(i, j)[0]) > delta)) {
				mask.at<uchar>(i, j + 1) = 255;
			}
		}

	gray = cv::Mat(mask);
	//dilate(gray, gray, kern, cv::Point(-1, -1), 5);
	//erode(gray, gray, kern, cv::Point(-1, -1), 5);

	dilate(gray, gray, cv::Mat(), cv::Point(-1, -1), CORE_RED_COLOR_FILTER);//cv::Point(-1, -1)
	//erode(gray, gray, cv::Mat(), cv::Point(-1, -1), 5);//cv::Point(-1, -1),
	//
	std::vector<cv::Vec4i> lines; // will hold the results of the detection
	HoughLinesP(gray, lines, 1, CV_PI / 180, 50, 50, 10); // runs the actual detection
	//medianBlur(gray,gray,5);
	Canny(gray, probabilistic_hough, m_settings.MinBoarderLight, m_settings.MaxBoarderLight, 3);
	//medianBlur(probabilistic_hough, probabilistic_hough,5);
	//int iterations = 10;
	//int j = iterations;
	//int64 t_cpp = 0;
	std::vector<std::vector<cv::Point> > contours;
	std::vector<std::vector<cv::Point>> squares;
	// find contours and store them all as a list
	cv::findContours(probabilistic_hough, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_TC89_KCOS);// RETR_LIST, CHAIN_APPROX_SIMPLE);
	std::vector<cv::Point> approx;
	// test each contour
	int maxWidthRibbonRect = boundingRect(contours[0]).width;
	for (size_t i = 0; i < contours.size(); i++)
	{
		// approximate contour with accuracy proportional to the contour perimeter
		approxPolyDP(contours[i], approx, arcLength(contours[i], true) * 0.02, true);
		// square contours should have 4 vertices after approximation
		double maxCosine = 0;
		if (approx.size() >= 4)
		{
			for (int j = 2; j < 5; j++)
			{
				// find the maximum cosine of the angle between joint edges
				double cosine = fabs(angle(approx[j % 4], approx[j - 2], approx[j - 1]));
				maxCosine += MAX(maxCosine, cosine);
			}
		}


		auto contArea = contourArea(approx);
		auto _rect = boundingRect(approx);
		if ((contArea > m_settings.MinSquareRibbon /*&& contArea < MAX_SQUARE_RIBBON*/) || _rect.width > m_settings.MinLengthRibbon) {
			if (maxCosine < m_settings.MinSummAngle) {//MIN_SUMM_ANGL
				squares.push_back(approx);
				if (maxWidthRibbonRect < _rect.width)
					maxWidthRibbonRect = _rect.width;
			}
		}
	}

	/*
	1 - Неизвестно
	2 - Прямая
	3 - Змейка
	*/
	//Вычисляем форму ограждений

	cv::Mat result = drawSquaresBoth(image, squares, lines);

	//--------------В найденной области находим устойчивые линии и осталяем только те что пересекаются с контурами-----------------------------
	std::vector<std::pair<double, cv::Vec4i>> ribbons_lines;//angle - line
	for (size_t i = 0; i < lines.size(); i++)
	{
		cv::Vec4i l = lines[i];
		auto dist_pts = distBetweenPoints(cv::Point(l[0], l[1]), cv::Point(l[2], l[3]));
		auto ang_line = static_cast<int>(l[0] == l[2] ? 89 : std::atan((std::abs(l[1] - l[3])) / std::abs(l[0] - l[2])) * 180 / 3.141);
		auto koef_b = l[1] - l[0] * std::tan(ang_line * _PI_ / 180);
		if (dist_pts > std::min(maxWidthRibbonRect, result.rows / m_settings.MinPartHeightFrame) && squares.size() > 0) {

			if (isIntersectionLineByRect(l, squares)) {

				ribbons_lines.push_back(std::pair<int, cv::Vec4i>(ang_line, l));
				//Show
				//line(result, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 3, LINE_AA);
				putText(result, tostr(ang_line) + " dist:" + tostr((int)dist_pts), cv::Point(l[0], l[1]), 1, 3, cv::Scalar(0, 0, 255));

			}
			//собираем ста-ку о всех линиях по змейке, из которых потом оставим только те что попадут в макс_рект зоны интереса
			if (ang_line <= m_settings.AnglSeparateHorizontalAndVerticalLines)
				HorizontalLineMAZE.push_back(RibbonLine(ang_line, koef_b, l));
			//
			if (ang_line > m_settings.AnglSeparateHorizontalAndVerticalLines)
				VerticalLineMAZE.push_back(RibbonLine(ang_line, koef_b, l));
		}

	}

	//находим структуру ограждения в зоне интереса
#ifdef OLD_VERSHION
	//Show
	//////////////////////////////////////////////old ver/////////////////////////////////////////////////////////////
	if (summ_rect.width > 0 && summ_rect.height > 0 && ribbons_lines.size() > 0) {
		int cnt_ZeroAngleRribbons = 0;
		int cnt_NoneZeroAngleRibbons = 0;
		int cnt_IntersectionNpneParallelRibbons = 0;
		std::vector<int> m_NoneZeroAgleRibbonsIndex;
		for (size_t i = 0; i < ribbons_lines.size(); i++)
		{
			if (ribbons_lines[i].first > 10) {
				cnt_NoneZeroAngleRibbons++;
				m_NoneZeroAgleRibbonsIndex.push_back(i);
			}
			else
				cnt_ZeroAngleRribbons++;
		}


		std::string lable_text = "UnKNOWN";

		if ((cnt_ZeroAngleRribbons > 0 && cnt_NoneZeroAngleRibbons == 0) ||
			(cnt_ZeroAngleRribbons >= m_settings.LimitRibbons && cnt_NoneZeroAngleRibbons >= m_settings.LimitRibbons)) {//если найдено несколько горизонтальных линий
			if (m_settings.ShowImageDebug)
				lable_text = "It's SNAKE";

			ShapeDetector.push_back(ShapeRibbons::Snake);
		}
		else
			if (cnt_NoneZeroAngleRibbons > 0 && cnt_ZeroAngleRribbons == 0) {//если не найдено несколько ни одной горизонтальной линии, но найдены несколь вертикальных
				if (m_settings.ShowImageDebug)
					lable_text = "It's LINE";

				ShapeDetector.push_back(ShapeRibbons::Line);
			}
			else
			{//lable_text ;
				//если не параллельные прямые есть и они пересекаются внутри разрешенной области - это змейка
				if (m_NoneZeroAgleRibbonsIndex.size() > 0)
					for (auto& m_index : m_NoneZeroAgleRibbonsIndex)
						for (size_t i = 0; i < ribbons_lines.size(); i++)
							if (i != m_index)
							{
								if (isIntersectionLinesInsideResRECT(result, ribbons_lines[i].second, ribbons_lines[m_index].second, summ_rect))
									cnt_IntersectionNpneParallelRibbons++;
							}
				////если найдены точки пересечения не горизонтальных линий внутри области интереса
				if (cnt_IntersectionNpneParallelRibbons > 0 && (cnt_ZeroAngleRribbons > 0 || cnt_NoneZeroAngleRibbons > 0)) {
					if (m_settings.ShowImageDebug)
						lable_text = "It's SNAKE";

					ShapeDetector.push_back(ShapeRibbons::Snake);
				}
				else {
					if (m_settings.ShowImageDebug)
						lable_text = "UnKNOWN";

					ShapeDetector.push_back(ShapeRibbons::UnKnown);
				}
			}
	}

#else
	//////////////////////////////////////////////new ver//////////////////////////////////////////////////////////////
	std::string lable_text = "UnKNOWN";
	if ((HorizontalLineMAZE.size() > m_settings.CntRightStructRibbons || VerticalLineMAZE.size() > m_settings.CntRightStructRibbons) &&
		ShapeDetector.size() <= m_settings.CntRightStructRibbons) {
		_ribbon_horizontal = GetRegionMaze(result, HorizontalLineMAZE, cv::Scalar(100, 100, 255), cv::Scalar(0, 0, 255), m_settings.ShowImageDebug);
		_ribbon_vertical = GetRegionMaze(result, VerticalLineMAZE, cv::Scalar(255, 100, 100), cv::Scalar(255, 0, 0), m_settings.ShowImageDebug);
		//
		auto _currentShape = GetStructRibbonsByIntersectionLine(result, summ_rect, ribbons_lines);
		auto square_horiz_to_vertical = _ribbon_horizontal.size.area() / _ribbon_vertical.size.area();
		auto square_vertical_to_horiz = _ribbon_vertical.size.area() / _ribbon_horizontal.size.area();
		auto horizont_square_to_all = (double)(_ribbon_horizontal.size.area()) / (double)(result.cols * result.rows);
		auto vertical_square_to_all = (double)(_ribbon_vertical.size.area()) / (double)(result.cols * result.rows);

		if (_currentShape == ShapeRibbons::Snake) {
			if (horizont_square_to_all > 0.25 && vertical_square_to_all < 0.25) {

				ResultLineMAZE = HorizontalLineMAZE;

				if (m_settings.ShowImageDebug)
					lable_text = "IT-s Snake\n";
				currShapeRibbons = ShapeRibbons::Snake;
				//
				//ofs << "IT-s Snake\n";
				ShapeDetector.push_back(currShapeRibbons);
			}
			else
				if (horizont_square_to_all < 0.25 && vertical_square_to_all > 0.25) {

					if (isPartsBigMaze(_ribbon_horizontal, _ribbon_vertical, 60)) {

						if (m_settings.ShowImageDebug)
							lable_text = "IT-s Snake 2\n";
						ResultLineMAZE = HorizontalLineMAZE;
						currShapeRibbons = ShapeRibbons::Snake;
						//чистим вертикальные линии не попавшие в зону горизонтальных
						/*for (int i = 0; i < VerticalLineMAZE.size(); i++) {
							if (isIntersectionLineByRect2(std::get<2>(VerticalLineMAZE[i]), _ribbon_horizontal))
								VerticalLineMAZE.erase(VerticalLineMAZE.begin() + i);

						}*/
						ResultLineMAZE.insert(ResultLineMAZE.end(), VerticalLineMAZE.begin(), VerticalLineMAZE.end());
						//
						ShapeDetector.push_back(currShapeRibbons);
					}
				}
				else
					if (horizont_square_to_all > 0.25 /*&& vertical_square_to_all > 0.25*/) {

						if (isPartsBigMaze(_ribbon_horizontal, _ribbon_vertical, 40)/* || (remember_summ_rect.height > 0 && remember_summ_rect.width > 0) )*/) {

							if (m_settings.ShowImageDebug)
								lable_text = "IT-s Big Maze - Snake\n";

							ResultLineMAZE = HorizontalLineMAZE;
							//чистим вертикальные линии не попавшие в зону горизонтальных
							/*for (int i = 0; i < VerticalLineMAZE.size(); i++) {
								if (isIntersectionLineByRect2(std::get<2>(VerticalLineMAZE[i]), _ribbon_horizontal))
									VerticalLineMAZE.erase(VerticalLineMAZE.begin() + i);

							}*/
							ResultLineMAZE.insert(ResultLineMAZE.end(), VerticalLineMAZE.begin(), VerticalLineMAZE.end());
							//
							//ofs << "IT-s Big Maze - Snake\n";
							currShapeRibbons = ShapeRibbons::Snake;
							//
							ShapeDetector.push_back(ShapeRibbons::Snake);
						}
						else {
							if (square_horiz_to_vertical > 1 && square_vertical_to_horiz < 1) {
								//ofs << "1Snake\n";
								ResultLineMAZE = HorizontalLineMAZE;
								if (m_settings.ShowImageDebug)
									lable_text = "1Snake\n";
								currShapeRibbons = ShapeRibbons::Snake;
								//
								ShapeDetector.push_back(currShapeRibbons);
							}
							else
								if ((int)square_horiz_to_vertical == 1 || (int)square_vertical_to_horiz == 1) {
									//ofs << "3UnKnown\n";
									if (m_settings.ShowImageDebug)
										lable_text = "3UnKnown\n";
									currShapeRibbons = ShapeRibbons::UnKnown;
									//
									ShapeDetector.push_back(currShapeRibbons);
								}

						}
					}
					else {
						//ofs << "5UnKnown\n";

						if (m_settings.ShowImageDebug)
							lable_text = "5UnKnown\n";
						currShapeRibbons = ShapeRibbons::UnKnown;
						//
						ShapeDetector.push_back(currShapeRibbons);
					}
		}
		//
		if (_currentShape == ShapeRibbons::Line) {

			ResultLineMAZE = VerticalLineMAZE;

			if (m_settings.ShowImageDebug)
				lable_text = "IT-s Line 2\n";
			currShapeRibbons = ShapeRibbons::Line;
			remember_summ_rect = _ribbon_vertical.boundingRect();
			//
			ShapeDetector.push_back(currShapeRibbons);
		}
		//
		if (_currentShape == ShapeRibbons::UnKnown) {
			if (horizont_square_to_all < 0.25 && vertical_square_to_all > 0.25) {
				ResultLineMAZE = VerticalLineMAZE;

				if (m_settings.ShowImageDebug)
					lable_text = "IT-s Line 2\n";
				currShapeRibbons = ShapeRibbons::Line;
				remember_summ_rect = _ribbon_vertical.boundingRect();
				//
				ShapeDetector.push_back(currShapeRibbons);
			}
			else
				if (horizont_square_to_all > 0.25) {
					if (isPartsBigMaze(_ribbon_horizontal, _ribbon_vertical, 40)/* || (remember_summ_rect.height > 0 && remember_summ_rect.width > 0) )*/) {

						if (m_settings.ShowImageDebug)
							lable_text = "IT-s 12Big Maze - Snake\n";
						ResultLineMAZE = HorizontalLineMAZE;
						//чистим вертикальные линии не попавшие в зону горизонтальных
						//for (int i = 0; i < VerticalLineMAZE.size(); i++) {
						//	if (isIntersectionLineByRect2(std::get<2>(VerticalLineMAZE[i]), _ribbon_horizontal))
						//		VerticalLineMAZE.erase(VerticalLineMAZE.begin() + i);
						//}
						ResultLineMAZE.insert(ResultLineMAZE.end(), VerticalLineMAZE.begin(), VerticalLineMAZE.end());
						//
						//ofs << "IT-s Big Maze - Snake\n";
						currShapeRibbons = ShapeRibbons::Snake;
						//
						ShapeDetector.push_back(currShapeRibbons);
					}
					else {

						ResultLineMAZE = HorizontalLineMAZE;
						if (m_settings.ShowImageDebug)
							lable_text = "11Snake\n";
						currShapeRibbons = ShapeRibbons::Snake;
						//
						ShapeDetector.push_back(currShapeRibbons);
					}

				}
				else {
					if (m_settings.ShowImageDebug)
						lable_text = "8UnKNOWN\n";
					currShapeRibbons = ShapeRibbons::UnKnown;
					//
					//ofs << "7UnKNOWN\n";
					//
					ShapeDetector.push_back(currShapeRibbons);
				}


		}
	}
	else
		if (VerticalLineMAZE.size() < 10 * m_settings.LimitRibbons && HorizontalLineMAZE.size() < 10 * m_settings.LimitRibbons) {
			//std::cout << "UnKNOWN\n";
			if (m_settings.ShowImageDebug)
				lable_text = "IT-s UnKnown";

			ShapeDetector.push_back(ShapeRibbons::UnKnown);
		}
#endif	

	//
	//if (m_res_rects.size() > m_settings.CntRightStructRibbons)
		//m_res_rects.clear();
	//
	//////////////////////////////////////////////new ver//////////////////////////////////////////////////////////////
	//

	if (ShapeDetector.size() > m_settings.CntRightStructRibbons) {

		//находим самый частый результат рассчетов змейки
		int _currShapeRibbons = GetHiglyLiklyResult(ShapeDetector);
		//если форма текущего рассчета отличается от предыдущего сохраняем в БД 
		if (_currShapeRibbons != prevShapeRibbons) {

			//auto AverageToSquareAllFrame = (double)(summ_rect.area()) / (double)(img.rows * img.cols);
			//if (AverageToSquareAllFrame > 0.20) {

			//	auto AverageToMaximumRatio = (double)(summ_rect.area()) / (double)(summ_rect_max.area());
			//	if (AverageToSquareAllFrame > 0.7)
			//		SaveStuctRibbonsToDB(summ_rect, currShapeRibbons);
			//	else
			//		if(currShapeRibbons == ShapeRibbons::Line || AverageToMaximumRatio > 0.8)
			//			SaveStuctRibbonsToDB(summ_rect_max, currShapeRibbons);
			//	
			//	//запоминаем текущую форму
			//	prevShapeRibbons = currShapeRibbons;
			//}
			////////////////////////////////////////new ver/////////////////////////////////////////////////////
			/////////////////////////////определяем формузмейки///////////////////////////
#ifdef OLD_VERSHION
			if (img.rows > 0 && img.cols > 0 && summ_rect_max.area() > 0) {
				auto AverageToSquareAllFrame = (double)(summ_rect.area()) / (double)(img.rows * img.cols);

				if (AverageToSquareAllFrame > 0.3) {
					auto AverageToMaximumRatio = (double)(summ_rect.area()) / (double)(summ_rect_max.area());
					//for snake
					if (currShapeRibbons == ShapeRibbons::Snake) {
						if (AverageToSquareAllFrame > 0.7 || AverageToMaximumRatio < 0.8)
							SaveStuctRibbonsToDB(summ_rect, currShapeRibbons);
						else
							if (AverageToMaximumRatio > 0.8)
								SaveStuctRibbonsToDB(summ_rect_max, currShapeRibbons);
					}
					//for line
					if (currShapeRibbons == ShapeRibbons::Line) {
						if (AverageToSquareAllFrame < 0.4 || AverageToMaximumRatio < 0.8)
							SaveStuctRibbonsToDB(summ_rect_max, currShapeRibbons);
						else
							if (AverageToMaximumRatio > 0.8)
								SaveStuctRibbonsToDB(summ_rect, currShapeRibbons);
					}
				}
				else {
					//std::cout << "Very small region - UnKnown\n";
					currShapeRibbons = ShapeRibbons::UnKnown;
					SaveStuctRibbonsToDB(cv::Rect(), currShapeRibbons);
				}
			}
#else

			//////////////////////////сохраняем в БД/////////////////////////////
			//for snake
			if (_currShapeRibbons == ShapeRibbons::Snake) {
				auto _ribbon_itog = GetRegionMaze(result, ResultLineMAZE, cv::Scalar(100, 255, 100), cv::Scalar(0, 255, 0), m_settings.ShowImageDebug);
				SaveStuctRibbonsNSquareToDB(GetNSquare(_ribbon_itog), _currShapeRibbons);

			}
			//for line
			if (_currShapeRibbons == ShapeRibbons::Line) {

				SaveStuctRibbonsToDB(remember_summ_rect, _currShapeRibbons);
			}
			//for UnKnown
			if (_currShapeRibbons == ShapeRibbons::UnKnown) {

				SaveStuctRibbonsToDB(cv::Rect(), _currShapeRibbons);
			}
#endif
			//
			//запоминаем текущую форму
			prevShapeRibbons = currShapeRibbons;
			//Show
			if (ResultLineMAZE.size() > 0 && m_settings.ShowImageDebug) {

				if (currShapeRibbons == ShapeRibbons::Snake)
					GetRegionMaze(result, ResultLineMAZE, cv::Scalar(100, 255, 100), cv::Scalar(0, 255, 0), m_settings.ShowImageDebug);
				//
				if (currShapeRibbons == ShapeRibbons::Line)
					rectangle(result, summ_rect, cv::Scalar(0, 255, 0));

				cv::putText(result, lable_text, cv::Point(50, 50), 1, 5, cv::Scalar(255, 100, 255));
			}
			//чистим результаты расчетов
			CleanStatisticsCalc();
		}
	}



	if (m_settings.ShowImageDebug) {
		//cv::Mat tmp_show = cv::Mat(result);
		cv::resize(result, result, cv::Size(result.cols / 2, result.rows / 2));
		cv::imshow("RIBBONS" + (m_thread ? tostr(m_thread->get_id()) : "withoutThread"), result);
	}

	//resize(image, image, Size(image.cols / 2, image.rows / 2));
	//imshow("w", image);

	//t = ((double)getTickCount() - t) / getTickFrequency();

}