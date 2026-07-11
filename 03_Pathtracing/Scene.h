#pragma once
#include <memory>
#include <Mat4.h>
#include <optional>
#include <Vec3.h>
#include <vector>
#include "IntersectableObject.h"

class Scene
{
private:
	std::vector<std::shared_ptr<const IntersectableObject>> sceneObjects;
	Vec3 backgroundColor;
  bool environment;
	Mat4 model;

	Vec3 traceLocalPath(const Ray& ray, int maxDepth) const;

public:
	Scene()
  : backgroundColor{0.0f,0.0f,0.0f}
  , environment{true}
	{}

	Scene(const Vec3& backgroundColor)
		: backgroundColor(backgroundColor)
    , environment{false}
	{ }

	void addObject(std::shared_ptr<const IntersectableObject> object);
	void setModel(const Mat4& model);
	Mat4 getModel() const;
	Vec3 getBackgroundcolor() const;
	std::vector<float> getTriangleData() const;
	std::optional<Intersection> intersect(const Ray& ray) const;
	Vec3 tracePath(const Ray& ray, int maxDepth) const;

	static Scene genPathTracingScene();
	static Scene genCornellBox();
  static Scene genCausticsScene();

};
