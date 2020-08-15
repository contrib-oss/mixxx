#pragma once

#include <QByteArray>
#include <QColor>
#include <QDataStream>
#include <QList>
#include <memory>

#include "track/cueinfo.h"
#include "track/taglib/trackmetadata_file.h"
#include "util/assert.h"
#include "util/types.h"

namespace mixxx {

class SeratoBeatGridNonTerminalMarker;
class SeratoBeatGridTerminalMarker;

typedef std::shared_ptr<SeratoBeatGridNonTerminalMarker> SeratoBeatGridNonTerminalMarkerPointer;
typedef std::shared_ptr<SeratoBeatGridTerminalMarker> SeratoBeatGridTerminalMarkerPointer;

class SeratoBeatGridNonTerminalMarker {
  public:
    SeratoBeatGridNonTerminalMarker(float positionSecs, quint32 beatsTillNextMarker)
            : m_positionSecs(positionSecs),
              m_beatsTillNextMarker(beatsTillNextMarker) {
    }
    ~SeratoBeatGridNonTerminalMarker() = default;

    QByteArray dumpID3() const;
    static SeratoBeatGridNonTerminalMarkerPointer parseID3(const QByteArray&);

    float positionSecs() const {
        return m_positionSecs;
    }

    float beatsTillNextMarker() const {
        return m_beatsTillNextMarker;
    }

  private:
    float m_positionSecs;
    quint32 m_beatsTillNextMarker;
};

inline QDebug operator<<(QDebug dbg, const SeratoBeatGridNonTerminalMarker& arg) {
    return dbg << "SeratoBeatGridNonTerminalMarker"
               << "PositionMillis =" << arg.positionSecs()
               << "BeatTillNextMarker = " << arg.beatsTillNextMarker();
}

class SeratoBeatGridTerminalMarker {
  public:
    SeratoBeatGridTerminalMarker(float positionSecs, float bpm)
            : m_positionSecs(positionSecs),
              m_bpm(bpm) {
    }
    ~SeratoBeatGridTerminalMarker() = default;

    QByteArray dumpID3() const;
    static mixxx::SeratoBeatGridTerminalMarkerPointer parseID3(const QByteArray&);

    float positionSecs() const {
        return m_positionSecs;
    }

    float bpm() const {
        return m_bpm;
    }

  private:
    float m_positionSecs;
    float m_bpm;
};

inline QDebug operator<<(QDebug dbg, const SeratoBeatGridTerminalMarker& arg) {
    return dbg << "SeratoBeatGridTerminalMarker"
               << "PositionMillis =" << arg.positionSecs()
               << "BPM =" << arg.bpm();
}

/// DTO for storing information from the SeratoBeatGrid tags used by the Serato
/// DJ Pro software.
///
/// This class includes functions for formatting and parsing SeratoBeatGrid
/// metadata according to the specification:
/// https://github.com/Holzhaus/serato-tags/blob/master/docs/serato_beatgrid.md
class SeratoBeatGrid final {
  public:
    SeratoBeatGrid() = default;
    explicit SeratoBeatGrid(
            SeratoBeatGridTerminalMarkerPointer pTerminalMarker,
            QList<SeratoBeatGridNonTerminalMarkerPointer> nonTerminalMarkers)
            : m_pTerminalMarker(pTerminalMarker),
              m_nonTerminalMarkers(std::move(nonTerminalMarkers)),
              m_footer(0) {
    }

    /// Parse a binary Serato repesentation of the beatgrid data from a
    /// `QByteArray` and write the results to the `SeratoBeatGrid` instance.
    /// The `fileType` parameter determines the exact format of the data being
    /// used.
    static bool parse(
            SeratoBeatGrid* seratoBeatGrid,
            const QByteArray& data,
            taglib::FileType fileType);

    /// Create a binary Serato repesentation of the beatgrid data suitable for
    /// `fileType` and dump it into a `QByteArray`. The content of that byte
    /// array can be used for round-trip tests or written to the appropriate
    /// tag to make it accessible to Serato.
    QByteArray dump(taglib::FileType fileType) const;

    bool isEmpty() const {
        return !m_pTerminalMarker && m_nonTerminalMarkers.isEmpty();
    }

    const QList<SeratoBeatGridNonTerminalMarkerPointer>& nonTerminalMarkers() const {
        return m_nonTerminalMarkers;
    }
    void setNonTerminalMarkers(QList<SeratoBeatGridNonTerminalMarkerPointer> nonTerminalMarkers) {
        m_nonTerminalMarkers = nonTerminalMarkers;
    }

    const SeratoBeatGridTerminalMarkerPointer terminalMarker() const {
        return m_pTerminalMarker;
    }
    void setTerminalMarker(SeratoBeatGridTerminalMarkerPointer pTerminalMarker) {
        m_pTerminalMarker = pTerminalMarker;
    }

    quint8 footer() const {
        return m_footer;
    }
    void setFooter(quint8 footer) {
        m_footer = footer;
    }

    QList<double> getBeatPositionsMillis(double trackLengthMillis, double timingOffsetMillis) const;

  private:
    static bool parseID3(
            SeratoBeatGrid* seratoBeatGrid,
            const QByteArray& data);
    static bool parseBase64Encoded(
            SeratoBeatGrid* seratoBeatGrid,
            const QByteArray& base64EncodedData);

    QByteArray dumpID3() const;
    QByteArray dumpBase64Encoded() const;

    SeratoBeatGridTerminalMarkerPointer m_pTerminalMarker;
    QList<SeratoBeatGridNonTerminalMarkerPointer> m_nonTerminalMarkers;
    quint8 m_footer;
};

inline bool operator==(const SeratoBeatGrid& lhs, const SeratoBeatGrid& rhs) {
    return (lhs.terminalMarker() == rhs.terminalMarker() &&
            lhs.nonTerminalMarkers() == rhs.nonTerminalMarkers());
}

inline bool operator!=(const SeratoBeatGrid& lhs, const SeratoBeatGrid& rhs) {
    return !(lhs == rhs);
}

inline QDebug operator<<(QDebug dbg, const SeratoBeatGrid& arg) {
    // TODO: Improve debug output
    return dbg << "number of markers ="
               << (arg.nonTerminalMarkers().length() +
                          (arg.terminalMarker() ? 1 : 0));
}

} // namespace mixxx

Q_DECLARE_TYPEINFO(mixxx::SeratoBeatGrid, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(mixxx::SeratoBeatGrid)
