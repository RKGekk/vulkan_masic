#include "matrix_animation.h"

KeyframeMatrixTranslation::KeyframeMatrixTranslation() : TimePos(0.0f), Translation(0.0f, 0.0f, 0.0f), Tangent(0.0f, 0.0f, 0.0f) {}

KeyframeMatrixTranslation::~KeyframeMatrixTranslation() {}

KeyframeMatrixScale::KeyframeMatrixScale() : TimePos(0.0f), Scale(1.0f, 1.0f, 1.0f) {}

KeyframeMatrixScale::~KeyframeMatrixScale() {}

KeyframeMatrixRotation::KeyframeMatrixRotation() : TimePos(0.0f), RotationQuat(1.0f, 0.0f, 0.0f, 0.0f) {}

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

void MatrixAnimation::Interpolate(float t, glm::mat4x4& transform) const {
	size_t sz1 = TranslationKeyframes.size();
	size_t sz2 = ScaleKeyframes.size();
	size_t sz3 = RotationKeyframes.size();
	if (!(sz1 || sz3)) {
		return;
	}

	glm::vec3 S(1.0f, 1.0f, 1.0f);
	glm::vec3 P(0.0f, 0.0f, 0.0f);
	glm::quat Q(1.0f, 0.0f, 0.0f, 0.0f);

	if (TranslationKeyframes.size() == 0u) {
	}
	else if (TranslationKeyframes.size() == 1u) {
		P = TranslationKeyframes.front().Translation;
	}
	else if (t >= TranslationKeyframes.rbegin()->TimePos) {
        P = TranslationKeyframes.back().Translation;
	}
	else {
		auto it1 = std::lower_bound(TranslationKeyframes.cbegin(), TranslationKeyframes.cend(), t, [](const auto it, float t) { return it.TimePos < t; });
		if (it1 == TranslationKeyframes.cbegin()) {
			P = it1->Translation;
		}
		else {
			auto it0 = std::prev(it1);

			float current_time_pos = it0->TimePos;
			float next_time_pos = it1->TimePos;
			float time_delta = next_time_pos - current_time_pos;
			float lerp_percent = 0.5f;
			if (time_delta > 0.0001f) {
				lerp_percent = (t - current_time_pos) / (next_time_pos - current_time_pos);
			}

			glm::vec3 p0 = it0->Translation;
			glm::vec3 p1 = it1->Translation;

			glm::vec3 t0 = it0->Tangent;
			glm::vec3 t1 = it1->Tangent;

			//P = glm::mix(p0, p1, lerp_percent);
			P = glm::hermite(p0, t0, p1, t1, lerp_percent);
		}
	}

	if (ScaleKeyframes.size() == 0u) {
        
	}
	else if (ScaleKeyframes.size() == 1u) {
        S = ScaleKeyframes.front().Scale;
	}
	else if (t >= ScaleKeyframes.rbegin()->TimePos) {
        S = ScaleKeyframes.back().Scale;
	}
	else {
		auto it1 = std::lower_bound(ScaleKeyframes.cbegin(), ScaleKeyframes.cend(), t, [](const auto it, float t) { return it.TimePos < t; });
		if (it1 == ScaleKeyframes.cbegin()) {
			S = it1->Scale;
		}
		else {
			auto it0 = std::prev(it1);

			float current_time_pos = it0->TimePos;
			float next_time_pos = it1->TimePos;
			float time_delta = next_time_pos - current_time_pos;
			float lerp_percent = 0.5f;
			if (time_delta > 0.0001f) {
				lerp_percent = (t - current_time_pos) / (next_time_pos - current_time_pos);
			}

			glm::vec3 s0 = it0->Scale;
			glm::vec3 s1 = it1->Scale;

			S = glm::mix(s0, s1, lerp_percent);
		}
	}

	if (RotationKeyframes.size() == 0u) {

	}
	else if (RotationKeyframes.size() == 1u) {
        Q = RotationKeyframes.front().RotationQuat;
	}
	else if (t >= RotationKeyframes.rbegin()->TimePos) {
        Q = RotationKeyframes.back().RotationQuat;
	}
	else {
		auto it1 = std::lower_bound(RotationKeyframes.cbegin(), RotationKeyframes.cend(), t, [](const auto it, float t) { return it.TimePos < t; });
		if (it1 == RotationKeyframes.cbegin()) {
			Q = it1->RotationQuat;
		}
		else {
			auto it0 = std::prev(it1);

			float current_time_pos = it0->TimePos;
			float next_time_pos = it1->TimePos;
			float time_delta = next_time_pos - current_time_pos;
			float lerp_percent = 0.5f;
			if (time_delta > 0.0001f) {
				lerp_percent = (t - current_time_pos) / (next_time_pos - current_time_pos);
			}

			glm::quat q0 = it0->RotationQuat;
			glm::quat q1 = it1->RotationQuat;

            Q = glm::slerp(q0, q1, lerp_percent);
		}
	}

    glm::mat4x4 Scale = glm::scale(S);
	glm::mat4x4 Rotate(Q);
	glm::mat4x4 Translate = glm::translate(P);
	glm::mat4x4 new_transform = Translate * Rotate * Scale;
    transform *= new_transform;
}