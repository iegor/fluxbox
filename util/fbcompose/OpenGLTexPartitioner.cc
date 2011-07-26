/** OpenGLTexPartitioner.cc file for the fluxbox compositor. */

// Copyright (c) 2011 Gediminas Liktaras (gliktaras at gmail dot com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


#include "OpenGLTexPartitioner.hh"

#include "OpenGLScreen.hh"

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
OpenGL2DTexturePartition::OpenGL2DTexturePartition(const OpenGLScreen &screen, bool swizzleAlphaToOne) :
    m_screen(screen) {

    m_display = (Display*)(screen.display());
    m_maxTextureSize = screen.maxTextureSize();
    m_pixmap = None;
    m_swizzleAlphaToOne = swizzleAlphaToOne;

    m_fullHeight = 0;
    m_fullWidth = 0;
}

// Destructor.
OpenGL2DTexturePartition::~OpenGL2DTexturePartition() { }


//--- MUTATORS -----------------------------------------------------------------

// Sets the texture's contents to the given pixmap.
void OpenGL2DTexturePartition::setPixmap(Pixmap pixmap, bool managePixmap, int width, int height, int depth) {
    // Handle the pixmap and its GC.
    if (m_pixmap) {
        XFreePixmap(m_display, m_pixmap);
        m_pixmap = None;
    }
    if (managePixmap) {
        m_pixmap = pixmap;
    }
    GC gc = XCreateGC(m_display, pixmap, 0, NULL);

    // Set partition's dimensions and partition that space.
    m_fullHeight = height;
    m_fullWidth = width;

    int unitHeight, unitWidth;
    std::vector<XRectangle> spaceParts = partitionSpace(0, 0, width, height, m_maxTextureSize, &unitWidth, &unitHeight);
    int totalUnits = unitHeight * unitWidth;

    // Adjust number of stored partitions.
    while ((size_t)(totalUnits) > m_partitions.size()) {
        TexturePart partition;
        partition.borders = 0;
        partition.texture = new OpenGL2DTexture(m_screen, m_swizzleAlphaToOne);
        m_partitions.push_back(partition);
    }
    while ((size_t)(totalUnits) < m_partitions.size()) {
        m_partitions.pop_back();
    }

    // Create texture partitions.
    if (totalUnits == 1) {
        m_partitions[0].borders = BORDER_ALL;
        m_partitions[0].texture->setPixmap(pixmap, false, m_fullWidth, m_fullHeight, false);
    } else {
        for (int i = 0; i < unitHeight; i++) {
            for (int j = 0; j < unitWidth; j++) {
                int idx = i * unitWidth + j;

                // Create partition's pixmap.
                Pixmap partPixmap = XCreatePixmap(m_display, m_screen.rootWindow().window(),
                                                  spaceParts[idx].width, spaceParts[idx].height, depth);
                XCopyArea(m_display, pixmap, partPixmap, gc, spaceParts[idx].x, spaceParts[idx].y,
                          spaceParts[idx].width, spaceParts[idx].height, 0, 0);

                // Set up the partition.
                m_partitions[idx].borders = getBorderBitfield(unitWidth, unitHeight, j, i);
                m_partitions[idx].texture->setPixmap(partPixmap, true, spaceParts[idx].width, spaceParts[idx].height, false);
            }
        }
    }

    // Cleanup.
    XFreeGC(m_display, gc);
}


//--- SUPPORTING FUNCTIONS -------------------------------------------------

// Returns the border bitfield of the given partition.
unsigned int FbCompositor::getBorderBitfield(int unitWidth, int unitHeight, int x, int y) {
    unsigned int borders = 0;
    borders |= ((y == 0) ? BORDER_NORTH : 0);
    borders |= ((x == 0) ? BORDER_WEST : 0);
    borders |= ((y == (unitHeight - 1)) ? BORDER_SOUTH : 0);
    borders |= ((x == (unitWidth - 1)) ? BORDER_EAST : 0);
    return borders;
}

// Space partitioning function.
std::vector<XRectangle> FbCompositor::partitionSpace(int x, int y, int width, int height, int maxPartitionSize,
                                                     int *unitWidth_return, int *unitHeight_return) {
    int unitHeight = ((height - 1) / maxPartitionSize) + 1;
    int unitWidth = ((width - 1) / maxPartitionSize) + 1;

    std::vector<XRectangle> partitions;
    for (int i = 0; i < unitHeight; i++) {
        for (int j = 0; j < unitWidth; j++) {
            int partX = x + j * maxPartitionSize;
            int partY = y + i * maxPartitionSize;
            int partHeight = ((i == (unitHeight - 1)) ? (height % maxPartitionSize) : maxPartitionSize);
            int partWidth = ((j == (unitWidth - 1)) ? (width % maxPartitionSize) : maxPartitionSize);

            XRectangle part = { partX, partY, partWidth, partHeight };
            partitions.push_back(part);
        }
    }

    if (unitWidth_return && unitHeight_return) {
        *unitWidth_return = unitWidth;
        *unitHeight_return = unitHeight;
    }

    return partitions;
}
