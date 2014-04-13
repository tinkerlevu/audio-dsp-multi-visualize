#ifndef GONIOMETERCONTROL_H_INCLUDED
#define GONIOMETERCONTROL_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "TomatlImageType.h"

//==============================================================================
/*
*/
class GoniometerControl : public Component
{
private:
	Image mContent;
	Image mBackground;
	AdmvAudioProcessor* mParentProcessor;

	void initLayers()
	{
		Graphics buffer(mContent);
		buffer.setColour(Colours::transparentBlack);
		buffer.fillAll();

		float sqrt2 = std::sqrt(2);

		Point<float> center(getWidth() / 2, getHeight() / 2);

		float lineScaleX = getWidth() / 2;
		float lineScaleY = getHeight() / 2;

		
		Line<float> leftDiag(movePoint<float>(center, -lineScaleX / sqrt2, -lineScaleY / sqrt2), movePoint<float>(center, +lineScaleX / sqrt2, +lineScaleY / sqrt2));
		Line<float> rightDiag(movePoint<float>(center, -lineScaleX / sqrt2, +lineScaleY / sqrt2), movePoint<float>(center, +lineScaleX / sqrt2, -lineScaleY / sqrt2));

		Graphics background(mBackground);
		background.setImageResamplingQuality(Graphics::ResamplingQuality::highResamplingQuality);
		background.setColour(Colours::black);
		background.fillAll();
		background.setColour(Colour::fromString("FF202020"));
		background.drawEllipse(0., 0., getWidth(), getHeight(), 1.5);
		
		
		background.drawLine(leftDiag);
		background.drawLine(rightDiag);
	}
public:

	template <typename T> Point<T> movePoint(Point<T> source, T x, T y)
	{
		Point<T> newPoint(source);

		newPoint.addXY(x, y);

		return newPoint;
	}

	GoniometerControl(AdmvAudioProcessor* parentPlugin)
	{
		setOpaque(true);
		setPaintingIsUnclipped(false);
		setSize(400, 400);
		mContent = Image(Image::ARGB, getWidth(), getHeight(), true);
		mBackground = Image(Image::RGB, getWidth(), getHeight(), true, TomatlImageType());
		mParentProcessor = parentPlugin;
		
		initLayers();
		
	}

	~GoniometerControl()
	{
	}

	void paint (Graphics& g)
	{
		Graphics buffer(mContent);

		Image::BitmapData pixels(mContent, Image::BitmapData::ReadWriteMode::readWrite);

		// TODO: split to several smaller methods
		for (int cn = 0; cn < mParentProcessor->getMaxStereoPairCount(); ++cn)
		{
			GonioPoints<double> segment = mParentProcessor->mGonioSegments[cn];

			double totalDistance = 0;
			Path p;

			if (segment.mLength <= 0)
			{
				continue;
			}

			double x = (segment.mData[0].first + 1.) * getWidth() / 2.;
			double y = (segment.mData[0].second + 1.) * getHeight() / 2.;

			p.startNewSubPath(x, y);

			std::vector<std::pair<double, double>> points;

			points.push_back(std::pair<double, double>(x, y));

			for (int i = 1; i < segment.mLength - 1; ++i)
			{
				double x = (segment.mData[i].first + 1.) * getWidth() / 2.;
				double y = (segment.mData[i].second + 1.) * getHeight() / 2.;

				std::pair<double, double> prev = points[points.size() - 1];

				double distance = std::sqrt(std::pow(prev.first - x, 2) + std::pow(prev.second - y, 2));
				totalDistance += distance;

				if (distance > 20)
				{
					//p.quadraticTo(prev.first, prev.second, x, y);
					//p.addLineSegment(Line<float>(prev.first, prev.second, x, y), 0.7);
					points.push_back(std::pair<double, double>(x, y));
				}
			}

			tomatl::draw::ColorARGB color;
			color.fromColor(mParentProcessor->getStereoPairColor(segment.mIndex));

			if (points.size() > 4)
			{
				for (int i = 0; i < points.size() - 3; i += 3)
				{
					std::pair<double, double> p1 = points[i + 0];
					std::pair<double, double> p2 = points[i + 1];
					std::pair<double, double> p3 = points[i + 2];
					std::pair<double, double> p4 = points[i + 3];

					tomatl::draw::Util::cubic_bezier(
						p1.first, p1.second,
						p2.first, p2.second,
						p3.first, p3.second,
						p4.first, p4.second,
						pixels, color);

					//pixels.setPixelColour(points[i].first, points[i].second, Colours::green);
					//p.lineTo(points[i].first, points[i].second);
					//p.quadraticTo(points[i].first, points[i].second, points[i + 1].first, points[i + 1].second);
				}
			}

			p.closeSubPath();

			buffer.setColour(mParentProcessor->getStereoPairColor(segment.mIndex));
			//buffer.strokePath(p.createPathWithRoundedCorners(20.), PathStrokeType(1.0f));
			//
			// TODO: fix custom multiply alphas
			//tomatl::draw::Util::multiplyAlphas(pixels, 0.95);
			mContent.multiplyAllAlphas(0.95);
		}

		g.drawImageAt(mBackground, 0, 0);
		g.drawImageAt(mContent, 0, 0);
	}

	void resized()
	{
		// This contol doesn't support resizing ATM
	}

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GoniometerControl)
};


#endif  // GONIOMETERCONTROL_H_INCLUDED
