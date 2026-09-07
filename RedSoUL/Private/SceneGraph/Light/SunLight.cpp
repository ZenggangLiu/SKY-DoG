/// Library headers
#include "Assert/RuntimeAssert.hpp"
#include "Math/Interpolator.hpp"
#include "Math/MathDefines.hpp"   /// ONE_PI, DEGREE_TO_RADIAN
#include "Math/MathUtilities.hpp" /// fast_sincos
#include "SceneGraph/MarkerTypeDepot.hpp"
#include "SceneGraph/SceneObject.hpp"
#include "SceneGraph/TransformMarker.hpp"
/// Self header
#include "SceneGraph/Light/SunLight.hpp"


DEFINE_MARKER_TYPE_INFO(
    SunLight, SunLight::create, SunLight::destroy, false);


float_3
SunLight::world_light_direction () const
{
    return m_transform_marker.world_forward_vector();
}


void
SunLight::set_day_time (
    const uint8_t hour,
    const uint8_t minute)
{
    RUNTIME_ASSERT(hour   <= 23, "Wrong hour set: [0, 23]!!");
    RUNTIME_ASSERT(minute <= 59, "Wrong minute set: [0, 59]!!");

    /// 更新朝向
    update_forward_vector(hour, minute);
    /// 更新辐射能Le(ωl)
    update_sun_Le();
}


ObjectMarker *
SunLight::create (
    SceneObject & marker_owner)
{

    SunLight * const new_marker = new SunLight(marker_owner);
    new_marker->set_day_time(10, 30);
    return (ObjectMarker*)new_marker;
}


bool
SunLight::destroy (
    ObjectMarker * & marker_object)
{
    RUNTIME_ASSERT(marker_object, "Marker can not be NULL!!");

    SunLight * const light = static_cast<SunLight*>(marker_object);
    delete light;
    marker_object = nullptr;
    return true;
}


SunLight::SunLight (
    SceneObject & marker_owner)
:
    SuperT(marker_owner, ms_type_info.marker_name_id(), LightType::SUN_LIGHT,
           float_3::ONE, 1.0f, true)
{

}


SunLight::~SunLight ()
{

}


void
SunLight::update_forward_vector (
    const uint8_t hour,
    const uint8_t minute)
{
    /// 1973年10月17日是一年中的第几天(从0开始)
    static constexpr uint16_t DAY_NUMBER_IN_YEAR = 289;
    /// { -sin(longitude), cos(longitude), 0.0f }, 对于Greenwich而言, longitude == 0
    static constexpr float_3 EAST_DIRECTION{ 0.0f, 1.0f, 0.0f };
    /// { -sin(latitude)*cos(longitude), -sin(latitude)*sin(longitude), cos(latitude) }
    /// 对于Greenwich而言, latitude == 51.4769, longitude == 0
    /// cos(latitude) = cos(51.4769) = 0.6905137449
    /// sin(latitude) = sin(51.4769) = 0.7233192712
    static constexpr float_3 NORTH_DIRECTION{ -0.7233192712f, 0.0f, 0.6905137442f };
    /// { cos(latitude)*cos(longitude), cos(latitude)*sin(longitude), sin(latitude) }
    /// 对于Greenwich而言, latitude == 51.4769, longitude == 0
    /// cos(latitude) = cos(51.4769) = 0.6905137449
    /// sin(latitude) = sin(51.4769) = 0.7233192712
    static constexpr float_3 UP_DIRECTION{ 0.6905137449f, 0.0f, 0.7233192712f };

    const float time_in_hours = (float)hour + (float)minute / 60.0f;

    /// 计算Gamma γ(弧度):
    ///      2*π                     hour - 12
    /// γ = ────── * (day_of_year + ────────────
    ///      365                         24
    /// 说明:
    /// day_of_year: 从0开始统计的指定日期在一年中是第几天
    const float gamma = 2.0f * ONE_PI / 365.0f
                      * (DAY_NUMBER_IN_YEAR + (time_in_hours - 12.0f) / 24.0f);

    /// 从γ可以计算时间方程(分钟):
    /// 时间方程 =
    ///     229.18 *
    ///     (0.000075 +
    ///      0.001868*cos( γ) – 0.032077*sin( γ) –
    ///      0.014615*cos(2γ) – 0.040849*sin(2γ))
    ///
    float cos_gamma, sin_gamma;
    MathUtility::fast_sincos(gamma, sin_gamma, cos_gamma);

    float cos_2gamma, sin_2gamma;
    MathUtility::fast_sincos(2.0f * gamma, sin_2gamma, cos_2gamma);

    float cos_3gamma, sin_3gamma;
    MathUtility::fast_sincos(3.0f * gamma, sin_3gamma, cos_3gamma);

    float equation_of_time =
        229.18f *
        (
            0.000075f +
            0.001868f * cos_gamma  - 0.032077f * sin_gamma -
            0.014615f * cos_2gamma - 0.040849f * sin_2gamma
        );

    /// 太阳Declination(弧度):
    /// Declination =
    ///     0.006918 –
    ///     0.399912*cos(  γ) + 0.070257*sin(  γ) –
    ///     0.006758*cos(2*γ) + 0.000907*sin(2*γ) –
    ///     0.002697*cos(3*γ) + 0.001480*sin(3*γ)
    ///
    float declination =
        0.006918f -
        0.399912f * cos_gamma  + 0.070257f * sin_gamma  -
        0.006758f * cos_2gamma + 0.000907f * sin_2gamma -
        0.002697f * cos_3gamma + 0.001480f * sin_3gamma;

    float time_in_mins = hour * 60.0f + minute;
    /// 计算真太阳时间(分钟)
    float true_solar_mins = MathUtility::repeat(time_in_mins + equation_of_time, 1440.0f);

    /// 计算小时角(度) := 真太阳时间 / 4 - 180
    float hour_angle_degs = true_solar_mins / 4.0f - 180.0f;
    /// 日下点经度与小时角相反
    float subsolar_longitude_degs = -hour_angle_degs;

    /// 在地心坐标中构造太阳方向:
    /// X: 经度0°, 纬度0°方向
    /// Y: 经度90°E, 纬度0°方向
    /// Z: 北极方向
    ///
    float cos_declination, sin_declination;
    MathUtility::fast_sincos(declination, sin_declination, cos_declination);

    /// 日下点的纬度就是太阳Declination, 经度由真太阳时得到
    float subsolar_longitude_rads = DEGREE_TO_RADIAN(subsolar_longitude_degs);
    float cos_subsolar_long, sin_subsolar_long;
    MathUtility::fast_sincos(subsolar_longitude_rads, sin_subsolar_long, cos_subsolar_long);

    const float_3 earth_to_subsolar_point = float_3::make(
        cos_declination * cos_subsolar_long,
        cos_declination * sin_subsolar_long,
        sin_declination);

    /// 将地心太阳方向投影到Greenwich本地坐标:
    /// SUN的方位:
    ///   上方
    ///   Y
    ///   ^   Z
    ///   |  / 北方
    ///   | /
    ///   |/      X
    ///   O-------> 东方
    ///
    /// 这样得到的Y分量就是cos(天顶角), 不需要再用反三角函数求θ。
    const float_3 sun_direction = float_3::make(
        -earth_to_subsolar_point.dot(EAST_DIRECTION),
        -earth_to_subsolar_point.dot(UP_DIRECTION),
        -earth_to_subsolar_point.dot(NORTH_DIRECTION));

    m_transform_marker.set_world_forward_vector(sun_direction);
}


void
SunLight::update_sun_Le ()
{
    /// 日出/日落时刻, SUN的颜色
    static constexpr float_3 SUNSET_COLOR{ 1.0f, 0.42f, 0.16f };
    /// 白天时SUN的颜色
    static constexpr float_3 DAY_LIGHT_COLOR{ 1.0f, 0.96f, 0.86f };

    /// SUN的辐射亮度 Le(ωl) := s*C_rgb * δ(ωi - ωl)
    /// s:为光的强度缩放(Intensity)
    /// C_rgb: 为光的色彩
    ///
    static constexpr float MAX_LIGHT_INTENSITY = 1.25f;

    /// Intensity为零时候的θ角(NOTE: 如果当前θ大于等于此角, Intensity为0): θ从+Y轴开始衡量
    static constexpr float ZERO_INTENSITY_COS_THETA = -0.1045284633f;  /// cos(96°)
    /// Intensity为100%时的θ角(NOTE: 如果当前θ小于等于此角, Intensity为100%)
    static constexpr float FULL_INTENSITY_COS_THETA = 0.2588190451f;   /// cos(75°)
    /// 地平线处的θ角(NOTE: 如果当前θ大于等于此角, Color为0)
    static constexpr float NIGHT_COLOR_COS_THETA    = 0.0f;            /// cos(0°)
    static constexpr float DAYLIGHT_COLOR_COS_THETA = 0.5735764364f;   /// cos(55°)
    /// SUN所在处的cos(θ)
    const float sun_cos_theta =
        /// 由于Forward方向从SUN出发, 而θ的衡量从物体出发。因此取反。
        MathUtility::clamp(-m_transform_marker.world_forward_vector().y, -1.0f, 1.0f);

    /// 计算Intensity缩放
    /// θ角越小, cos(θ)越大。
    /// 因此直接用cos(θ)来为Intensity插值:
    /// - cos(θ) >= FULL_INTENSITY_COS_THETA时, Intensity为100%
    /// - cos(θ) <= ZERO_INTENSITY_COS_THETA时, Intensity为0%
    ///   上方
    ///   Y
    ///   ^   100%
    ///   |  /       * 0%
    ///   |θ/     *
    ///   |/  *
    ///   O-------> 东方
    const float intensity_scale = MathUtility::smooth_step(
        ZERO_INTENSITY_COS_THETA, FULL_INTENSITY_COS_THETA, sun_cos_theta);

    const float color_scale = MathUtility::smooth_step(
        NIGHT_COLOR_COS_THETA, DAYLIGHT_COLOR_COS_THETA, sun_cos_theta);

    /// 更新s*C_rgb
    set_intensity(MAX_LIGHT_INTENSITY * intensity_scale);
    set_color(Interpolator::linear(SUNSET_COLOR, DAY_LIGHT_COLOR, color_scale));
}
