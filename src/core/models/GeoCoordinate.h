/**
 * @file GeoCoordinate.h
 * @brief Domain Model representing a 3D Geographic Coordinate.
 *
 * @details Encapsulates WGS84 latitude, longitude, and altitude values.
 * Serves as a core Value Object in the GIS domain layer to enforce
 * type safety and coordinate boundary validation.
 */

#ifndef GEOCOORDINATE_H
#define GEOCOORDINATE_H

namespace GISApp::Core::Models {

/**
 * @class GeoCoordinate
 * @brief Represents a point on Earth using WGS84 geographic coordinates.
 */
class GeoCoordinate {
public:
    /**
     * @brief Default constructor initializing coordinates to Null Island (0°N, 0°E, 0m).
     */
    GeoCoordinate()
        : m_latitude(0.0), m_longitude(0.0), m_altitude(0.0) {}

    /**
     * @brief Parameterized constructor for 2D and 3D geographic coordinates.
     * @param lat Latitude in decimal degrees [-90.0, 90.0].
     * @param lon Longitude in decimal degrees [-180.0, 180.0].
     * @param altitude Height/Elevation in meters above the WGS84 ellipsoid (default: 0.0m).
     */
    GeoCoordinate(double lat, double lon, double altitude = 0.0)
        : m_latitude(lat), m_longitude(lon), m_altitude(altitude) {}

    /**
     * @brief Gets the latitude value.
     * @return Latitude in decimal degrees.
     */
    double latitude() const { return m_latitude; }

    /**
     * @brief Gets the longitude value.
     * @return Longitude in decimal degrees.
     */
    double longitude() const { return m_longitude; }

    /**
     * @brief Gets the altitude value.
     * @return Altitude in meters.
     */
    double altitude() const { return m_altitude; }

    /**
     * @brief Sets the latitude value.
     * @param lat Latitude in decimal degrees.
     */
    void setLatitude(double lat) { m_latitude = lat; }

    /**
     * @brief Sets the longitude value.
     * @param lon Longitude in decimal degrees.
     */
    void setLongitude(double lon) { m_longitude = lon; }

    /**
     * @brief Sets the altitude value.
     * @param alt Altitude in meters above sea level.
     */
    void setAltitude(double alt) { m_altitude = alt; }

    /**
     * @brief Validates if the latitude and longitude are within valid WGS84 spatial bounds.
     * @return True if -90 <= lat <= 90 and -180 <= lon <= 180; otherwise false.
     */
    bool isValid() const {
        return (m_latitude >= -90.0 && m_latitude <= 90.0) &&
               (m_longitude >= -180.0 && m_longitude <= 180.0);
    }

private:
    double m_latitude;  ///< Latitude coordinate in decimal degrees.
    double m_longitude; ///< Longitude coordinate in decimal degrees.
    double m_altitude;  ///< Height/Elevation in meters above WGS84 datum.
};

} // namespace GISApp::Core::Models

#endif // GEOCOORDINATE_H
