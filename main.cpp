/*
 * Sifteo SDK Example.
 */

#include <sifteo.h>
#include "assets.gen.h"
using namespace Sifteo;

static Metadata M = Metadata()
    .title("GroupSynth")
    .package("com.sifteo.sdk.synth", "1.0")
    .icon(Icon)
    .cubeRange(1, CUBE_ALLOCATION);

static const CubeID cube0 = 0;
static const CubeID cube1 = 0;
static const CubeID cube2 = 0;
static VideoBuffer vid[CUBE_ALLOCATION];
static TiltShakeRecognizer motion[CUBE_ALLOCATION];

//A baseline for what the background color will be
//This changes dynamically
int startCount = 0;


class SensorListener {
public:
    struct Counter {
        unsigned touch;
        unsigned neighborAdd;
        unsigned neighborRemove;
    } counters[CUBE_ALLOCATION];

    void install()
    {
        Events::neighborAdd.set(&SensorListener::onNeighborAdd, this);
        Events::neighborRemove.set(&SensorListener::onNeighborRemove, this);
        Events::cubeTouch.set(&SensorListener::onTouch, this);
        Events::cubeConnect.set(&SensorListener::onConnect, this);

        // Handle already-connected cubes
        for (CubeID cube : CubeSet::connected())
            onConnect(cube);
    }

private:
    void onConnect(unsigned id)
    {
        CubeID cube(id);
        uint64_t hwid = cube.hwID();

        bzero(counters[id]);
        LOG("Cube %d connected\n", id);

        vid[id].initMode(BG0_ROM);
        vid[id].attach(id);
        motion[id].attach(id);


        // Draw initial state for all sensors
        onTouch(cube);
        drawNeighbors(cube);
    }

    void onTouch(unsigned id)
    {
        startCount++;
        for (CubeID cube : CubeSet::connected())
            drawNeighbors(cube);
    }


    void onNeighborRemove(unsigned firstID, unsigned firstSide, unsigned secondID, unsigned secondSide)
    {
        LOG("Neighbor Remove: %02x:%d - %02x:%d\n", firstID, firstSide, secondID, secondSide);

        if (firstID < arraysize(counters)) {
            counters[firstID].neighborRemove++;
            drawNeighbors(firstID);
        }
        if (secondID < arraysize(counters)) {
            counters[secondID].neighborRemove++;
            drawNeighbors(secondID);
        }
    }

    void onNeighborAdd(unsigned firstID, unsigned firstSide, unsigned secondID, unsigned secondSide)
    {
        LOG("Neighbor Add: %02x:%d - %02x:%d\n", firstID, firstSide, secondID, secondSide);

        if (firstID < arraysize(counters)) {
            counters[firstID].neighborAdd++;
            drawNeighbors(firstID);
        }
        if (secondID < arraysize(counters)) {
            counters[secondID].neighborAdd++;
            drawNeighbors(secondID);
        }
    }

    void drawNeighbors(CubeID cube)
    {
        Neighborhood nb(cube);

        BG0ROMDrawable &draw = vid[cube].bg0rom;

        unsigned bg;
        int adj = startCount;

        //Count number of adjacencies
        adj += (nb.hasCubeAt(TOP) ? 1: 0);
        adj += (nb.hasCubeAt(LEFT) ? 1: 0);
        adj += (nb.hasCubeAt(BOTTOM) ? 1: 0); 
        adj += (nb.hasCubeAt(RIGHT) ? 1: 0);

        switch(adj%5){
            case 0: bg = BG0ROMDrawable::BLACK;
                    break;
            case 1: bg = BG0ROMDrawable::RED;
                    break;
            case 2: bg = BG0ROMDrawable::ORANGE;
                    break;
            case 3: bg = BG0ROMDrawable::GREEN;
                    break;
            case 4: bg = BG0ROMDrawable::PURPLE;
                    break;
        }

        draw.fill(vec(0,0), vec(16,16), bg | BG0ROMDrawable::SOLID_FG);

        LOG("%d Num neighbors: %d\n", (unsigned)cube, adj);

        drawSideIndicator(draw, nb, vec( 1,  0), vec(14,  1), TOP);
        drawSideIndicator(draw, nb, vec( 0,  1), vec( 1, 14), LEFT);
        drawSideIndicator(draw, nb, vec( 1, 15), vec(14,  1), BOTTOM);
        drawSideIndicator(draw, nb, vec(15,  1), vec( 1, 14), RIGHT);
    }

    static void drawSideIndicator(BG0ROMDrawable &draw, Neighborhood &nb,
        Int2 topLeft, Int2 size, Side s)
    {
        unsigned nbColor = draw.ORANGE;
        draw.fill(topLeft, size,
            nbColor | (nb.hasNeighborAt(s) ? draw.SOLID_FG : draw.SOLID_BG));
    }
};


void main()
{
    static SensorListener sensors;

    sensors.install();


    while (1) {
        System::paint();
    }
}
