/* Webcamoid, webcam capture application.
 * Copyright (C) 2016  Gonzalo Exequiel Pedone
 *
 * Webcamoid is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Webcamoid is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Webcamoid. If not, see <http://www.gnu.org/licenses/>.
 *
 * Web-Site: http://webcamoid.github.io/
 */

#include <QDataStream>
#include <QDebug>
#include <QMetaEnum>
#include <QVector>
#include <QtMath>

#include "akaudiocaps.h"
#include "akcaps.h"

class SampleFormats
{
    public:
        AkAudioCaps::SampleFormat format;
        AkAudioCaps::SampleType type;
        int bps;
        int endianness;

        static inline const QVector<SampleFormats> &formats()
        {
            static const QVector<SampleFormats> sampleFormats {
                {AkAudioCaps::SampleFormat_none , AkAudioCaps::SampleType_unknown,  0, Q_BYTE_ORDER   },
                {AkAudioCaps::SampleFormat_s8   , AkAudioCaps::SampleType_int    ,  8, Q_BYTE_ORDER   },
                {AkAudioCaps::SampleFormat_u8   , AkAudioCaps::SampleType_uint   ,  8, Q_BYTE_ORDER   },
                {AkAudioCaps::SampleFormat_s16le, AkAudioCaps::SampleType_int    , 16, Q_LITTLE_ENDIAN},
                {AkAudioCaps::SampleFormat_s16be, AkAudioCaps::SampleType_int    , 16, Q_BIG_ENDIAN   },
                {AkAudioCaps::SampleFormat_u16le, AkAudioCaps::SampleType_uint   , 16, Q_LITTLE_ENDIAN},
                {AkAudioCaps::SampleFormat_u16be, AkAudioCaps::SampleType_uint   , 16, Q_BIG_ENDIAN   },
                {AkAudioCaps::SampleFormat_s32le, AkAudioCaps::SampleType_int    , 32, Q_LITTLE_ENDIAN},
                {AkAudioCaps::SampleFormat_s32be, AkAudioCaps::SampleType_int    , 32, Q_BIG_ENDIAN   },
                {AkAudioCaps::SampleFormat_u32le, AkAudioCaps::SampleType_uint   , 32, Q_LITTLE_ENDIAN},
                {AkAudioCaps::SampleFormat_u32be, AkAudioCaps::SampleType_uint   , 32, Q_BIG_ENDIAN   },
                {AkAudioCaps::SampleFormat_s64le, AkAudioCaps::SampleType_int    , 64, Q_LITTLE_ENDIAN},
                {AkAudioCaps::SampleFormat_s64be, AkAudioCaps::SampleType_int    , 64, Q_BIG_ENDIAN   },
                {AkAudioCaps::SampleFormat_u64le, AkAudioCaps::SampleType_uint   , 64, Q_LITTLE_ENDIAN},
                {AkAudioCaps::SampleFormat_u64be, AkAudioCaps::SampleType_uint   , 64, Q_BIG_ENDIAN   },
                {AkAudioCaps::SampleFormat_fltle, AkAudioCaps::SampleType_float  , 32, Q_LITTLE_ENDIAN},
                {AkAudioCaps::SampleFormat_fltbe, AkAudioCaps::SampleType_float  , 32, Q_BIG_ENDIAN   },
                {AkAudioCaps::SampleFormat_dblle, AkAudioCaps::SampleType_float  , 64, Q_LITTLE_ENDIAN},
                {AkAudioCaps::SampleFormat_dblbe, AkAudioCaps::SampleType_float  , 64, Q_BIG_ENDIAN   },
            };

            return sampleFormats;
        }

        static inline const SampleFormats *byFormat(AkAudioCaps::SampleFormat format)
        {
            for (auto &format_: formats())
                if (format_.format == format)
                    return &format_;

            return &formats().front();
        }

        static inline const SampleFormats *byType(AkAudioCaps::SampleType type)
        {
            for (auto &format: formats())
                if (format.type == type)
                    return &format;

            return &formats().front();
        }

        static inline const SampleFormats *byBps(int bps)
        {
            for (auto &format: formats())
                if (format.bps == bps)
                    return &format;

            return &formats().front();
        }

        static inline const SampleFormats *byEndianness(int endianness)
        {
            for (auto &format: formats())
                if (format.endianness == endianness)
                    return &format;

            return &formats().front();
        }

        template<typename T>
        static inline T alignUp(const T &value, const T &align)
        {
            return (value + align - 1) & ~(align - 1);
        }
};

class SpeakerPositions
{
    public:
        AkAudioCaps::Position position;
        qreal azimuth;
        qreal elevation;

        static inline const QVector<SpeakerPositions> &positions()
        {
            /* Speakers positions based on:
             *
             * https://mediaarea.net/AudioChannelLayout
             */
            static const QVector<SpeakerPositions> positions {
                {AkAudioCaps::Position_unknown            ,    0.0,   0.0},
                {AkAudioCaps::Position_FrontCenter        ,    0.0,   0.0},
                {AkAudioCaps::Position_FrontLeft          ,   45.0,   0.0},
                {AkAudioCaps::Position_FrontRight         ,  -45.0,   0.0},
                {AkAudioCaps::Position_BackCenter         ,  180.0,   0.0},
                {AkAudioCaps::Position_BackLeft           ,  150.0,   0.0},
                {AkAudioCaps::Position_BackRight          , -150.0,   0.0},
                {AkAudioCaps::Position_FrontLeftOfCenter  ,   15.0,   0.0},
                {AkAudioCaps::Position_FrontRightOfCenter ,  -15.0,   0.0},
                {AkAudioCaps::Position_WideLeft           ,   60.0,   0.0},
                {AkAudioCaps::Position_WideRight          ,  -60.0,   0.0},
                {AkAudioCaps::Position_SideLeft           ,   90.0,   0.0},
                {AkAudioCaps::Position_SideRight          ,  -90.0,   0.0},
                {AkAudioCaps::Position_LowFrequency1      ,   60.0, -30.0},
                {AkAudioCaps::Position_LowFrequency2      ,  -60.0, -30.0},
                {AkAudioCaps::Position_TopCenter          ,    0.0,  90.0},
                {AkAudioCaps::Position_TopFrontCenter     ,    0.0,  45.0},
                {AkAudioCaps::Position_TopFrontLeft       ,   45.0,  45.0},
                {AkAudioCaps::Position_TopFrontRight      ,  -45.0,  45.0},
                {AkAudioCaps::Position_TopBackCenter      ,  180.0,  45.0},
                {AkAudioCaps::Position_TopBackLeft        ,  135.0,  45.0},
                {AkAudioCaps::Position_TopBackRight       , -135.0,  45.0},
                {AkAudioCaps::Position_TopSideLeft        ,   90.0,  45.0},
                {AkAudioCaps::Position_TopSideRight       ,  -90.0,  45.0},
                {AkAudioCaps::Position_BottomFrontCenter  ,    0.0, -30.0},
                {AkAudioCaps::Position_BottomFrontLeft    ,   45.0, -30.0},
                {AkAudioCaps::Position_BottomFrontRight   ,  -45.0, -30.0},
                {AkAudioCaps::Position_StereoLeft         ,  110.0,   0.0},
                {AkAudioCaps::Position_StereoRight        , -110.0,   0.0},
                {AkAudioCaps::Position_SurroundDirectLeft ,   90.0,   0.0},
                {AkAudioCaps::Position_SurroundDirectRight,  -90.0,   0.0},
            };

            return positions;
        }

        static inline const SpeakerPositions *byPosition(AkAudioCaps::Position position)
        {
            for (auto &position_: positions())
                if (position_.position == position)
                    return &position_;

            return &positions().front();
        }
};

#define LAYOUT_MONO          AkAudioCaps::Position_FrontCenter
#define LAYOUT_STEREO        AkAudioCaps::Position_FrontLeft, \
                             AkAudioCaps::Position_FrontRight
#define LAYOUT_DOWNMIX       AkAudioCaps::Position_StereoLeft, \
                             AkAudioCaps::Position_StereoRight
#define LAYOUT_2P1           LAYOUT_STEREO, AkAudioCaps::Position_LowFrequency1
#define LAYOUT_3P0           LAYOUT_STEREO, AkAudioCaps::Position_FrontCenter
#define LAYOUT_3P0_BACK      LAYOUT_STEREO, AkAudioCaps::Position_BackCenter
#define LAYOUT_3P1           LAYOUT_3P0, AkAudioCaps::Position_LowFrequency1
#define LAYOUT_4P0           LAYOUT_3P0, AkAudioCaps::Position_BackCenter
#define LAYOUT_QUAD          LAYOUT_STEREO, \
                             AkAudioCaps::Position_BackLeft, \
                             AkAudioCaps::Position_BackRight
#define LAYOUT_QUAD_SIDE     LAYOUT_STEREO, \
                             AkAudioCaps::Position_SideLeft, \
                             AkAudioCaps::Position_SideRight
#define LAYOUT_4P1           LAYOUT_4P0, AkAudioCaps::Position_LowFrequency1
#define LAYOUT_5P0           LAYOUT_3P0, AkAudioCaps::Position_BackLeft, \
                             AkAudioCaps::Position_BackRight
#define LAYOUT_5P0_SIDE      LAYOUT_3P0, AkAudioCaps::Position_SideLeft, \
                             AkAudioCaps::AkAudioCaps::Position_SideRight
#define LAYOUT_5P1           LAYOUT_5P0, AkAudioCaps::Position_LowFrequency1
#define LAYOUT_5P1_SIDE      LAYOUT_5P0_SIDE, \
                             AkAudioCaps::Position_LowFrequency1
#define LAYOUT_6P0           LAYOUT_5P0_SIDE, AkAudioCaps::Position_BackCenter
#define LAYOUT_6P0_FRONT     LAYOUT_QUAD_SIDE, \
                             AkAudioCaps::Position_FrontLeftOfCenter, \
                             AkAudioCaps::Position_FrontRightOfCenter
#define LAYOUT_HEXAGONAL     LAYOUT_5P0, AkAudioCaps::Position_BackCenter
#define LAYOUT_6P1           LAYOUT_5P1_SIDE, AkAudioCaps::Position_BackCenter
#define LAYOUT_6P1_BACK      LAYOUT_5P1, AkAudioCaps::Position_BackCenter
#define LAYOUT_6P1_FRONT     LAYOUT_6P0_FRONT, \
                             AkAudioCaps::Position_LowFrequency1
#define LAYOUT_7P0           LAYOUT_5P0_SIDE, \
                             AkAudioCaps::Position_BackLeft, \
                             AkAudioCaps::Position_BackRight
#define LAYOUT_7P0_FRONT     LAYOUT_5P0_SIDE, \
                             AkAudioCaps::Position_FrontLeftOfCenter, \
                             AkAudioCaps::Position_FrontRightOfCenter
#define LAYOUT_7P1           LAYOUT_5P1_SIDE, \
                             AkAudioCaps::Position_BackLeft, \
                             AkAudioCaps::Position_BackRight
#define LAYOUT_7P1_WIDE      LAYOUT_5P1_SIDE, \
                             AkAudioCaps::Position_FrontLeftOfCenter, \
                             AkAudioCaps::Position_FrontRightOfCenter
#define LAYOUT_7P1_WIDE_BACK LAYOUT_5P1, \
                             AkAudioCaps::Position_FrontLeftOfCenter, \
                             AkAudioCaps::Position_FrontRightOfCenter
#define LAYOUT_OCTAGONAL     LAYOUT_7P0, AkAudioCaps::Position_BackCenter
#define LAYOUT_HEXADECAGONAL LAYOUT_OCTAGONAL, \
                             AkAudioCaps::Position_WideLeft, \
                             AkAudioCaps::Position_WideRight, \
                             AkAudioCaps::Position_TopBackLeft, \
                             AkAudioCaps::Position_TopBackRight, \
                             AkAudioCaps::Position_TopBackCenter, \
                             AkAudioCaps::Position_TopFrontCenter, \
                             AkAudioCaps::Position_TopFrontLeft, \
                             AkAudioCaps::Position_TopFrontRight

class ChannelLayouts
{
    public:
        AkAudioCaps::ChannelLayout layout;
        QVector<AkAudioCaps::Position> channels;
        QString description;

        static inline const QVector<ChannelLayouts> &layouts()
        {
            static const QVector<ChannelLayouts> channelLayouts {
                {AkAudioCaps::Layout_none         , {}                    , "none"         },
                {AkAudioCaps::Layout_mono         , {LAYOUT_MONO}         , "mono"          },
                {AkAudioCaps::Layout_stereo       , {LAYOUT_STEREO}       , "stereo"        },
                {AkAudioCaps::Layout_2p1          , {LAYOUT_DOWNMIX}      , "2.1"           },
                {AkAudioCaps::Layout_3p0          , {LAYOUT_2P1}          , "3.0"           },
                {AkAudioCaps::Layout_3p0_back     , {LAYOUT_3P0}          , "3.0(back)"     },
                {AkAudioCaps::Layout_3p1          , {LAYOUT_3P0_BACK}     , "3.1"           },
                {AkAudioCaps::Layout_4p0          , {LAYOUT_3P1}          , "4.0"           },
                {AkAudioCaps::Layout_quad         , {LAYOUT_4P0}          , "quad"          },
                {AkAudioCaps::Layout_quad_side    , {LAYOUT_QUAD}         , "quad(side)"    },
                {AkAudioCaps::Layout_4p1          , {LAYOUT_QUAD_SIDE}    , "4.1"           },
                {AkAudioCaps::Layout_5p0          , {LAYOUT_4P1}          , "5.0"           },
                {AkAudioCaps::Layout_5p0_side     , {LAYOUT_5P0}          , "5.0(side)"     },
                {AkAudioCaps::Layout_5p1          , {LAYOUT_5P0_SIDE}     , "5.1"           },
                {AkAudioCaps::Layout_5p1_side     , {LAYOUT_5P1}          , "5.1(side)"     },
                {AkAudioCaps::Layout_6p0          , {LAYOUT_5P1_SIDE}     , "6.0"           },
                {AkAudioCaps::Layout_6p0_front    , {LAYOUT_6P0}          , "6.0(front)"    },
                {AkAudioCaps::Layout_hexagonal    , {LAYOUT_6P0_FRONT}    , "hexagonal"     },
                {AkAudioCaps::Layout_6p1          , {LAYOUT_HEXAGONAL}    , "6.1"           },
                {AkAudioCaps::Layout_6p1_back     , {LAYOUT_6P1}          , "6.1(back)"     },
                {AkAudioCaps::Layout_6p1_front    , {LAYOUT_6P1_BACK}     , "6.1(front)"    },
                {AkAudioCaps::Layout_7p0          , {LAYOUT_6P1_FRONT}    , "7.0"           },
                {AkAudioCaps::Layout_7p0_front    , {LAYOUT_7P0}          , "7.0(front)"    },
                {AkAudioCaps::Layout_7p1          , {LAYOUT_7P0_FRONT}    , "7.1"           },
                {AkAudioCaps::Layout_7p1_wide     , {LAYOUT_7P1}          , "7.1(wide)"     },
                {AkAudioCaps::Layout_7p1_wide_back, {LAYOUT_7P1_WIDE}     , "7.1(wide-back)"},
                {AkAudioCaps::Layout_octagonal    , {LAYOUT_7P1_WIDE_BACK}, "octagonal"     },
                {AkAudioCaps::Layout_hexadecagonal, {LAYOUT_OCTAGONAL}    , "hexadecagonal" },
                {AkAudioCaps::Layout_downmix      , {LAYOUT_HEXADECAGONAL}, "downmix"       },
            };

            return channelLayouts;
        }

        static inline const ChannelLayouts *byLayout(AkAudioCaps::ChannelLayout layout)
        {
            for (auto &layout_: layouts())
                if (layout_.layout == layout)
                    return &layout_;

            return &layouts().front();
        }

        static inline const ChannelLayouts *byChannelCount(int channels)
        {
            for (auto &layout: layouts())
                if (layout.channels.size() == channels)
                    return &layout;

            return &layouts().front();
        }

        static inline const ChannelLayouts *byDescription(const QString &description)
        {
            for (auto &layout: layouts())
                if (layout.description == description)
                    return &layout;

            return &layouts().front();
        }
};

class AkAudioCapsPrivate
{
    public:
        AkAudioCaps::SampleFormat m_format {AkAudioCaps::SampleFormat_none};
        AkAudioCaps::ChannelLayout m_layout {AkAudioCaps::Layout_none};
        size_t m_planeSize {0};
        QVector<size_t> m_offset;
        int m_rate {0};
        int m_samples {false};
        int m_align {1};
        bool m_planar {false};

        void updateParams();
};

AkAudioCaps::AkAudioCaps(QObject *parent):
    QObject(parent)
{
    this->d = new AkAudioCapsPrivate();
}

AkAudioCaps::AkAudioCaps(SampleFormat format,
                         ChannelLayout layout,
                         int rate,
                         int samples,
                         bool planar,
                         int align)
{
    this->d = new AkAudioCapsPrivate();
    this->d->m_format = format;
    this->d->m_layout = layout;
    this->d->m_rate = rate;
    this->d->m_samples = samples;
    this->d->m_planar = planar;
    this->d->m_align = align;
    this->d->updateParams();
}

AkAudioCaps::AkAudioCaps(const AkCaps &caps)
{
    this->d = new AkAudioCapsPrivate();

    if (caps.mimeType() == "audio/x-raw")
        this->update(caps);
}

AkAudioCaps::AkAudioCaps(const AkAudioCaps &other):
    QObject()
{
    this->d = new AkAudioCapsPrivate();
    this->d->m_format = other.d->m_format;
    this->d->m_layout = other.d->m_layout;
    this->d->m_rate = other.d->m_rate;
    this->d->m_samples = other.d->m_samples;
    this->d->m_planar = other.d->m_planar;
    this->d->m_align = other.d->m_align;
    this->d->m_planeSize = other.d->m_planeSize;
    this->d->m_offset = other.d->m_offset;

    for (auto &property: other.dynamicPropertyNames())
        this->setProperty(property, other.property(property));
}

AkAudioCaps::~AkAudioCaps()
{
    delete this->d;
}

AkAudioCaps &AkAudioCaps::operator =(const AkAudioCaps &other)
{
    if (this != &other) {
        this->d->m_format = other.d->m_format;
        this->d->m_layout = other.d->m_layout;
        this->d->m_rate = other.d->m_rate;
        this->d->m_samples = other.d->m_samples;
        this->d->m_planar = other.d->m_planar;
        this->d->m_align = other.d->m_align;
        this->d->m_planeSize = other.d->m_planeSize;
        this->d->m_offset = other.d->m_offset;

        this->clear();

        for (auto &property: other.dynamicPropertyNames())
            this->setProperty(property, other.property(property));

        this->d->updateParams();
    }

    return *this;
}

AkAudioCaps &AkAudioCaps::operator =(const AkCaps &caps)
{
    if (caps.mimeType() == "audio/x-raw") {
        this->update(caps);
    } else {
        this->d->m_format = SampleFormat_none;
        this->d->m_layout = Layout_none;
        this->d->m_rate = 0;
        this->d->m_samples = 0;
        this->d->m_planar = false;
        this->d->m_align = 1;
        this->d->m_planeSize = 1;
    }

    return *this;
}

bool AkAudioCaps::operator ==(const AkAudioCaps &other) const
{
    if (this->dynamicPropertyNames() != other.dynamicPropertyNames())
        return false;

    for (auto &property: this->dynamicPropertyNames())
        if (this->property(property) != other.property(property))
            return false;

    return this->d->m_format == other.d->m_format
            && this->d->m_layout == other.d->m_layout
            && this->d->m_rate == other.d->m_rate
            && this->d->m_samples == other.d->m_samples
            && this->d->m_planar == other.d->m_planar
            && this->d->m_align == other.d->m_align;
}

bool AkAudioCaps::operator !=(const AkAudioCaps &other) const
{
    return !(*this == other);
}

AkAudioCaps::operator AkCaps() const
{
    AkCaps caps("audio/x-raw");
    caps.setProperty("format", this->d->m_format);
    caps.setProperty("layout" , this->d->m_layout);
    caps.setProperty("rate", this->d->m_rate);
    caps.setProperty("samples"   , this->d->m_samples);
    caps.setProperty("planar"   , this->d->m_planar);
    caps.setProperty("align" , this->d->m_align);

    return caps;
}

AkAudioCaps::operator bool() const
{
    auto af = SampleFormats::byFormat(this->d->m_format);
    auto layouts = ChannelLayouts::byLayout(this->d->m_layout);

    if (!af || !layouts)
        return false;

    if (af->format == SampleFormat_none || layouts->layout == Layout_none)
        return false;

    return this->d->m_rate > 0;
}

AkAudioCaps::SampleFormat AkAudioCaps::format() const
{
    return this->d->m_format;
}

AkAudioCaps::ChannelLayout AkAudioCaps::layout() const
{
    return this->d->m_layout;
}

int AkAudioCaps::bps() const
{
    return AkAudioCaps::bitsPerSample(this->d->m_format);
}

int AkAudioCaps::channels() const
{
    return AkAudioCaps::channelCount(this->d->m_layout);
}

int AkAudioCaps::rate() const
{
    return this->d->m_rate;
}

int &AkAudioCaps::rate()
{
    return this->d->m_rate;
}

int AkAudioCaps::samples() const
{
    return this->d->m_samples;
}

bool AkAudioCaps::planar() const
{
    return this->d->m_planar;
}

int AkAudioCaps::align() const
{
    return this->d->m_align;
}

size_t AkAudioCaps::frameSize() const
{
    auto af = SampleFormats::byFormat(this->d->m_format);

    if (!af)
        return 0;

    return size_t(this->d->m_planar? this->channels(): 1)
            * this->d->m_planeSize;
}

const QVector<AkAudioCaps::Position> AkAudioCaps::positions() const
{
    auto layouts = ChannelLayouts::byLayout(this->d->m_layout);

    if (!layouts)
        return {};

    if (layouts->layout == Layout_none)
        return {};

    return layouts->channels;
}

AkAudioCaps AkAudioCaps::fromMap(const QVariantMap &caps)
{
    AkAudioCaps audioCaps;

    if (!caps.contains("mimeType") || caps["mimeType"] != "audio/x-raw")
        return audioCaps;

    for (auto it = caps.begin(); it != caps.end(); it++) {
        auto value = it.value();

        if (it.key() == "mimeType")
            continue;

        audioCaps.setProperty(it.key().toStdString().c_str(), value);
    }

    return audioCaps;
}

QVariantMap AkAudioCaps::toMap() const
{
    QVariantMap map {
        {"mimeType", "audio/x-raw"     },
        {"format"  , this->d->m_format },
        {"layout"  , this->d->m_layout },
        {"rate"    , this->d->m_rate   },
        {"samples" , this->d->m_samples},
        {"planar"  , this->d->m_planar },
        {"align"   , this->d->m_align  },
    };

    for (auto &property: this->dynamicPropertyNames()) {
        auto key = QString::fromUtf8(property.constData());
        map[key] = this->property(property);
    }

    return map;
}

AkAudioCaps &AkAudioCaps::update(const AkCaps &caps)
{
    if (caps.mimeType() != "audio/x-raw")
        return *this;

    this->clear();

    for (auto &property: caps.dynamicPropertyNames()) {
        int i = this->metaObject()->indexOfProperty(property);

        if (this->metaObject()->property(i).isWritable())
            this->setProperty(property, caps.property(property));
    }

    this->d->updateParams();

    return *this;
}

size_t AkAudioCaps::planeOffset(int plane) const
{
    return this->d->m_offset[plane];
}

int AkAudioCaps::planes() const
{
    auto af = SampleFormats::byFormat(this->d->m_format);
    auto layouts = ChannelLayouts::byLayout(this->d->m_layout);

    if (!af || !layouts)
        return 0;

    return this->d->m_planar? layouts->channels.size(): 1;
}

size_t AkAudioCaps::planeSize() const
{
    return this->d->m_planeSize;
}

int AkAudioCaps::bitsPerSample(SampleFormat sampleFormat)
{
    return SampleFormats::byFormat(sampleFormat)->bps;
}

int AkAudioCaps::bitsPerSample(const QString &sampleFormat)
{
    return AkAudioCaps::bitsPerSample(AkAudioCaps::sampleFormatFromString(sampleFormat));
}

QString AkAudioCaps::sampleFormatToString(SampleFormat sampleFormat)
{
    AkAudioCaps caps;
    int formatIndex = caps.metaObject()->indexOfEnumerator("SampleFormat");
    QMetaEnum formatEnum = caps.metaObject()->enumerator(formatIndex);
    QString format(formatEnum.valueToKey(sampleFormat));
    format.remove("SampleFormat_");

    return format;
}

AkAudioCaps::SampleFormat AkAudioCaps::sampleFormatFromString(const QString &sampleFormat)
{
    AkAudioCaps caps;
    QString format = "SampleFormat_" + sampleFormat;
    int formatIndex = caps.metaObject()->indexOfEnumerator("SampleFormat");
    QMetaEnum formatEnum = caps.metaObject()->enumerator(formatIndex);
    int formatInt = formatEnum.keyToValue(format.toStdString().c_str());

    return static_cast<SampleFormat>(formatInt);
}

AkAudioCaps::SampleFormat AkAudioCaps::sampleFormatFromProperties(SampleType type,
                                                                  int bps,
                                                                  int endianness)
{
    for (auto &sampleFormat: SampleFormats::formats())
        if (sampleFormat.type == type
            && sampleFormat.bps == bps
            && sampleFormat.endianness == endianness) {
            return sampleFormat.format;
        }

    return AkAudioCaps::SampleFormat_none;
}

bool AkAudioCaps::sampleFormatProperties(SampleFormat sampleFormat,
                                         SampleType *type,
                                         int *bps,
                                         int *endianness)
{
    auto format = SampleFormats::byFormat(sampleFormat);

    if (!format)
        return false;

    if (type)
        *type = format->type;

    if (bps)
        *bps = format->bps;

    if (endianness)
        *endianness = format->endianness;

    return true;
}

bool AkAudioCaps::sampleFormatProperties(const QString &sampleFormat,
                                         SampleType *type,
                                         int *bps,
                                         int *endianness)
{
    return AkAudioCaps::sampleFormatProperties(AkAudioCaps::sampleFormatFromString(sampleFormat),
                                               type,
                                               bps,
                                               endianness);
}

AkAudioCaps::SampleType AkAudioCaps::sampleType(SampleFormat sampleFormat)
{
    return SampleFormats::byFormat(sampleFormat)->type;
}

AkAudioCaps::SampleType AkAudioCaps::sampleType(const QString &sampleFormat)
{
    return AkAudioCaps::sampleType(AkAudioCaps::sampleFormatFromString(sampleFormat));
}

QString AkAudioCaps::channelLayoutToString(ChannelLayout channelLayout)
{
    return ChannelLayouts::byLayout(channelLayout)->description;
}

AkAudioCaps::ChannelLayout AkAudioCaps::channelLayoutFromString(const QString &channelLayout)
{
    return ChannelLayouts::byDescription(channelLayout)->layout;
}

int AkAudioCaps::channelCount(ChannelLayout channelLayout)
{
    return ChannelLayouts::byLayout(channelLayout)->channels.size();
}

int AkAudioCaps::channelCount(const QString &channelLayout)
{
    return ChannelLayouts::byDescription(channelLayout)->channels.size();
}

int AkAudioCaps::endianness(SampleFormat sampleFormat)
{
    return SampleFormats::byFormat(sampleFormat)->endianness;
}

int AkAudioCaps::endianness(const QString &sampleFormat)
{
    return AkAudioCaps::endianness(AkAudioCaps::sampleFormatFromString(sampleFormat));
}

AkAudioCaps::ChannelLayout AkAudioCaps::defaultChannelLayout(int channelCount)
{
    return ChannelLayouts::byChannelCount(channelCount)->layout;
}

QString AkAudioCaps::defaultChannelLayoutString(int channelCount)
{
    return ChannelLayouts::byChannelCount(channelCount)->description;
}

const QVector<AkAudioCaps::Position> &AkAudioCaps::positions(ChannelLayout channelLayout)
{
    return ChannelLayouts::byLayout(channelLayout)->channels;
}

AkAudioCaps::SpeakerPosition AkAudioCaps::position(Position position)
{
    auto sp = SpeakerPositions::byPosition(position);

    if (!sp)
        return {};

    return {sp->azimuth, sp->elevation};
}

AkAudioCaps::SpeakerPosition AkAudioCaps::position(int channel) const
{
    auto positions = AkAudioCaps::positions(this->d->m_layout);

    return AkAudioCaps::position(positions.at(channel));
}

void AkAudioCaps::setFormat(SampleFormat format)
{
    if (this->d->m_format == format)
        return;

    this->d->m_format = format;
    this->d->updateParams();
    emit this->formatChanged(format);
}

void AkAudioCaps::setLayout(ChannelLayout layout)
{
    if (this->d->m_layout == layout)
        return;

    this->d->m_layout = layout;
    this->d->updateParams();
    emit this->layoutChanged(layout);
}

void AkAudioCaps::setRate(int rate)
{
    if (this->d->m_rate == rate)
        return;

    this->d->m_rate = rate;
    emit this->rateChanged(rate);
}

void AkAudioCaps::setSamples(int samples)
{
    if (this->d->m_samples == samples)
        return;

    this->d->m_samples = samples;
    this->d->updateParams();
    emit this->samplesChanged(samples);
}

void AkAudioCaps::setPlanar(bool planar)
{
    if (this->d->m_planar == planar)
        return;

    this->d->m_planar = planar;
    this->d->updateParams();
    emit this->planarChanged(planar);
}

void AkAudioCaps::setAlign(int align)
{
    if (this->d->m_align == align)
        return;

    this->d->m_align = align;
    this->d->updateParams();
    emit this->alignChanged(align);
}

void AkAudioCaps::resetFormat()
{
    this->setFormat(SampleFormat_none);
}

void AkAudioCaps::resetLayout()
{
    this->setLayout(Layout_none);
}

void AkAudioCaps::resetRate()
{
    this->setRate(0);
}

void AkAudioCaps::resetSamples()
{
    this->setSamples(0);
}

void AkAudioCaps::resetPlanar()
{
    this->setPlanar(false);
}

void AkAudioCaps::resetAlign()
{
    this->setAlign(1);
}

void AkAudioCaps::clear()
{
    for (auto &property: this->dynamicPropertyNames())
        this->setProperty(property.constData(), QVariant());
}

void AkAudioCapsPrivate::updateParams()
{
    auto af = SampleFormats::byFormat(this->m_format);
    auto layouts = ChannelLayouts::byLayout(this->m_layout);

    if (!af || !layouts) {
        this->m_planeSize = 0;
        this->m_offset.clear();

        return;
    }

    if (this->m_planar) {
        this->m_planeSize =
                SampleFormats::alignUp(size_t(af->bps
                                                 * this->m_samples
                                                 / 8),
                                       size_t(this->m_align));
    } else {
        this->m_planeSize =
                SampleFormats::alignUp(size_t(af->bps
                                                 * layouts->channels.size()
                                                 * this->m_samples
                                                 / 8),
                                       size_t(this->m_align));
    }

    this->m_offset.clear();
    size_t nplanes = this->m_planar? size_t(layouts->channels.size()): 1;

    for (size_t i = 0; i < nplanes; i++)
        this->m_offset << i * this->m_planeSize;
}

qreal operator -(const AkAudioCaps::SpeakerPosition &pos1,
                  const AkAudioCaps::SpeakerPosition &pos2)
{
    auto a1 = pos1.first * M_PI / 180.0;
    auto e1 = pos1.second * M_PI / 180.0;

    auto x1 = qCos(e1) * qCos(a1);
    auto y1 = qCos(e1) * qSin(a1);
    auto z1 = qSin(e1);

    auto a2 = pos2.first * M_PI / 180.0;
    auto e2 = pos2.second * M_PI / 180.0;

    auto x2 = qCos(e2) * qCos(a2);
    auto y2 = qCos(e2) * qSin(a2);
    auto z2 = qSin(e2);

    auto dx = x1 - x2;
    auto dy = y1 - y2;
    auto dz = z1 - z2;

    return qSqrt(dx * dx + dy * dy + dz * dz);
}

QDebug operator <<(QDebug debug, const AkAudioCaps &caps)
{
    debug.nospace() << "AkAudioCaps("
                    << "format="
                    << caps.format()
                    << ",layout="
                    << caps.layout()
                    << ",rate="
                    << caps.rate()
                    << ",samples="
                    << caps.samples()
                    << ",planar="
                    << caps.planar()
                    << ",align="
                    << caps.align();

    QStringList properties;

    for (auto &property: caps.dynamicPropertyNames())
        properties << QString::fromUtf8(property.constData());

    properties.sort();

    for (auto &property: properties)
        debug.nospace() << ","
                        << property.toStdString().c_str()
                        << "="
                        << caps.property(property.toStdString().c_str());

    debug.nospace() << ")";

    return debug.space();
}

QDebug operator <<(QDebug debug, const AkAudioCaps::SampleFormat &format)
{
    debug.nospace() << AkAudioCaps::sampleFormatToString(format).toStdString().c_str();

    return debug.space();
}

QDebug operator <<(QDebug debug, const AkAudioCaps::SampleType &sampleType)
{
    AkAudioCaps caps;
    int sampleTypeIndex = caps.metaObject()->indexOfEnumerator("SampleType");
    QMetaEnum sampleTypeEnum = caps.metaObject()->enumerator(sampleTypeIndex);
    QString sampleTypeStr(sampleTypeEnum.valueToKey(sampleType));
    sampleTypeStr.remove("SampleType_");
    debug.nospace() << sampleTypeStr.toStdString().c_str();

    return debug.space();
}

QDebug operator <<(QDebug debug, const AkAudioCaps::Position &position)
{
    AkAudioCaps caps;
    int positionIndex = caps.metaObject()->indexOfEnumerator("Position");
    QMetaEnum positionEnum = caps.metaObject()->enumerator(positionIndex);
    QString positionStr(positionEnum.valueToKey(position));
    positionStr.remove("Position_");
    debug.nospace() << positionStr.toStdString().c_str();

    return debug.space();
}

QDebug operator <<(QDebug debug, const AkAudioCaps::ChannelLayout &layout)
{
    debug.nospace() << AkAudioCaps::channelLayoutToString(layout).toStdString().c_str();

    return debug.space();
}

QDataStream &operator >>(QDataStream &istream, AkAudioCaps &caps)
{
    int nProperties;
    istream >> nProperties;

    for (int i = 0; i < nProperties; i++) {
        QByteArray key;
        QVariant value;
        istream >> key;
        istream >> value;

        caps.setProperty(key.toStdString().c_str(), value);
    }

    return istream;
}

QDataStream &operator <<(QDataStream &ostream, const AkAudioCaps &caps)
{
    QVariantMap staticProperties {
        {"format" , caps.format() },
        {"layout" , caps.layout() },
        {"rate"   , caps.rate()   },
        {"samples", caps.samples()},
        {"planar" , caps.planar() },
        {"align"  , caps.align()  },
    };

    int nProperties =
            staticProperties.size() + caps.dynamicPropertyNames().size();
    ostream << nProperties;

    for (auto &key: caps.dynamicPropertyNames()) {
        ostream << key;
        ostream << caps.property(key);
    }

    return ostream;
}

#include "moc_akaudiocaps.cpp"
