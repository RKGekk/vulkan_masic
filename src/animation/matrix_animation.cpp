#include "matrix_animation.h"

KeyframeMatrixTranslation::KeyframeMatrixTranslation() : TimePos(0.0f), Translation(0.0f, 0.0f, 0.0f), inTangent(0.0f, 0.0f, 0.0f), outTangent(0.0f, 0.0f, 0.0f) {}

KeyframeMatrixTranslation::~KeyframeMatrixTranslation() {}

KeyframeMatrixScale::KeyframeMatrixScale() : TimePos(0.0f), Scale(1.0f, 1.0f, 1.0f) {}

KeyframeMatrixScale::~KeyframeMatrixScale() {}

KeyframeMatrixRotation::KeyframeMatrixRotation() : TimePos(0.0f), RotationQuat(1.0f, 0.0f, 0.0f, 0.0f), inTangent(1.0f, 0.0f, 0.0f, 0.0f), outTangent(1.0f, 0.0f, 0.0f, 0.0f) {}

KeyframeMatrixRotation::~KeyframeMatrixRotation() {}

bool operator<(const KeyframeMatrixTranslation& kf1, const KeyframeMatrixTranslation& kf2) {
	return kf1.TimePos < kf2.TimePos;
}

bool operator<(const KeyframeMatrixScale& kf1, const KeyframeMatrixScale& kf2) {
	return kf1.TimePos < kf2.TimePos;
}

bool operator<(const KeyframeMatrixRotation& kf1, const KeyframeMatrixRotation& kf2) {
	return kf1.TimePos < kf2.TimePos;
}

void MatrixAnimation::InterpolateTime(float t, glm::mat4x4& transform, float blend_factor) const {
	size_t sz1 = TranslationKeyframes.size();
	size_t sz2 = ScaleKeyframes.size();
	size_t sz3 = RotationKeyframes.size();
	if (!(sz1 || sz3)) {
		return;
	}

	glm::vec3 P(0.0f, 0.0f, 0.0f);
	glm::vec3 S(1.0f, 1.0f, 1.0f);
	glm::quat Q(1.0f, 0.0f, 0.0f, 0.0f);

	{
		glm::quat orientation;
    	glm::vec3 scale;
    	glm::vec3 translation;
    	glm::vec3 skew;
    	glm::vec4 perspective;
    	if (glm::decompose(transform, scale, orientation, translation, skew, perspective)) {
			P = translation;
			S = scale;
			Q = orientation;
		}
	}

	if (TranslationKeyframes.size() == 0u) {
	}
	else if (TranslationKeyframes.size() == 1u) {
		P = glm::lerp(P, TranslationKeyframes.front().Translation, blend_factor);
	}
	else if (t >= TranslationKeyframes.rbegin()->TimePos) {
        P = glm::lerp(P, TranslationKeyframes.back().Translation, blend_factor);
	}
	else {
		auto it1 = std::lower_bound(TranslationKeyframes.cbegin(), TranslationKeyframes.cend(), t, [](const auto it, float t) { return it.TimePos < t; });
		if (it1 == TranslationKeyframes.cbegin()) {
			P = glm::lerp(P, it1->Translation, blend_factor);
		}
		else if(std::prev(it1)->InterpolationType == KeyFrameInterpolationType::STEP) {
			P = glm::lerp(P, std::prev(it1)->Translation, blend_factor);
		}
		else {
			auto it0 = std::prev(it1);

			float current_time_pos = it0->TimePos;
			float next_time_pos = it1->TimePos;
			float time_delta = next_time_pos - current_time_pos;
			float lerp_percent = 0.5f;
			if (time_delta > 0.0001f) {
				lerp_percent = (t - current_time_pos) / time_delta;
			}

			glm::vec3 p0 = it0->Translation;
			glm::vec3 p1 = it1->Translation;

			if(it0->InterpolationType == KeyFrameInterpolationType::LINEAR) {
				//P = glm::mix(p0, p1, lerp_percent);
				P = glm::lerp(P, glm::lerp(p0, p1, lerp_percent), blend_factor);
			}
			else if(it0->InterpolationType == KeyFrameInterpolationType::CUBICSPLINE) {
				// glm::vec3 t0 = it0->Tangent;
				// glm::vec3 t1 = it1->Tangent;
				// Multiply the tangents by the frame's time duration because of how decouples physical velocity from keyframe spacing.The primary reasons for this requirement involve mathematical unit cancellation, normalized curve shapes, and maintaining consistent animation speeds
				glm::vec3 t0 = it0->inTangent * time_delta;
				glm::vec3 t1 = it0->outTangent * time_delta;
				//glm::vec3 t0 = it0->inTangent;
				//glm::vec3 t1 = it0->outTangent;

				P = glm::lerp(P, glm::hermite(p0, t0, p1, t1, lerp_percent), blend_factor);
			}
		}
	}

	if (ScaleKeyframes.size() == 0u) {
	}
	else if (ScaleKeyframes.size() == 1u) {
        S = glm::lerp(S, ScaleKeyframes.front().Scale, blend_factor);
	}
	else if (t >= ScaleKeyframes.rbegin()->TimePos) {
        S = glm::lerp(S, ScaleKeyframes.back().Scale, blend_factor);
	}
	else {
		auto it1 = std::lower_bound(ScaleKeyframes.cbegin(), ScaleKeyframes.cend(), t, [](const auto it, float t) { return it.TimePos < t; });
		if (it1 == ScaleKeyframes.cbegin()) {
			S = glm::lerp(S, it1->Scale, blend_factor);
		}
		else {
			auto it0 = std::prev(it1);

			float current_time_pos = it0->TimePos;
			float next_time_pos = it1->TimePos;
			float time_delta = next_time_pos - current_time_pos;
			float lerp_percent = 0.5f;
			if (time_delta > 0.0001f) {
				lerp_percent = (t - current_time_pos) / time_delta;
			}

			glm::vec3 s0 = it0->Scale;
			glm::vec3 s1 = it1->Scale;

			S = glm::lerp(S, glm::mix(s0, s1, lerp_percent), blend_factor);
		}
	}

	if (RotationKeyframes.size() == 0u) {

	}
	else if (RotationKeyframes.size() == 1u) {
        Q = glm::slerp(Q, RotationKeyframes.front().RotationQuat, blend_factor);
	}
	else if (t >= RotationKeyframes.rbegin()->TimePos) {
        Q = glm::slerp(Q, RotationKeyframes.back().RotationQuat, blend_factor);
	}
	else {
		auto it1 = std::lower_bound(RotationKeyframes.cbegin(), RotationKeyframes.cend(), t, [](const auto it, float t) { return it.TimePos < t; });
		if (it1 == RotationKeyframes.cbegin()) {
			Q = glm::slerp(Q, it1->RotationQuat, blend_factor);
		}
		else if(std::prev(it1)->InterpolationType == KeyFrameInterpolationType::STEP) {
			Q = glm::slerp(Q, std::prev(it1)->RotationQuat, blend_factor);
		}
		else {
			auto it0 = std::prev(it1);

			float current_time_pos = it0->TimePos;
			float next_time_pos = it1->TimePos;
			float time_delta = next_time_pos - current_time_pos;
			float lerp_percent = 0.5f;
			if (time_delta > 0.0001f) {
				lerp_percent = (t - current_time_pos) / time_delta;
			}

			glm::quat q0 = it0->RotationQuat;
			glm::quat q1 = it1->RotationQuat;

			if (glm::dot(q0, q1) < 0.0f) {
        		q1 = -q1; // Invert quaternion to prevent long-way wrapping
    		}

			if(it0->InterpolationType == KeyFrameInterpolationType::LINEAR) {
				Q = glm::slerp(Q, glm::slerp(q0, q1, lerp_percent), blend_factor);
			}
			else if(it0->InterpolationType == KeyFrameInterpolationType::CUBICSPLINE) {
				// glm::quat t0 = it0->Tangent;
				// glm::quat t1 = it1->Tangent;
				// Multiply the tangents by the frame's time duration because of how decouples physical velocity from keyframe spacing.The primary reasons for this requirement involve mathematical unit cancellation, normalized curve shapes, and maintaining consistent animation speeds
				glm::quat t0 = it0->inTangent * time_delta;
				glm::quat t1 = it0->outTangent * time_delta;

				// Spline accumulation breaks unit length; normalization is mandatory
				Q = glm::slerp(Q, glm::normalize(glm::hermite(q0, t0, q1, t1, lerp_percent)), blend_factor);
			}
		}
	}

    glm::mat4x4 Scale = glm::scale(S);
	glm::mat4x4 Rotate(Q);
	glm::mat4x4 Translate = glm::translate(P);
	glm::mat4x4 new_transform = Translate * Rotate * Scale;
	transform = new_transform;
    //transform *= new_transform;
}

glm::mat4x4 MatrixAnimation::InterpolateTime(float t) const {
	glm::mat4x4 transform = glm::mat4x4(1.0f);
	InterpolateTime(t, transform);
	return transform;
}

void MatrixAnimation::InterpolateNormValue(float v, glm::mat4x4& transform) const {
	float t = GetTotalAnimationTime() * v;
	InterpolateTime(t, transform);
}

glm::mat4x4 MatrixAnimation::InterpolateNormValue(float v) const {
	glm::mat4x4 transform = glm::mat4x4(1.0f);
	float t = GetTotalAnimationTime() * v;
	InterpolateTime(t, transform);
	return transform;
}

float MatrixAnimation::GetTotalAnimationTime() const {
	float t1 = RotationKeyframes.size() > 0u ? RotationKeyframes.back().TimePos : 0.0f;
	float t2 = TranslationKeyframes.size() > 0u ? TranslationKeyframes.back().TimePos : 0.0f;
    return t1 > t2 ? t1 : t2;
}