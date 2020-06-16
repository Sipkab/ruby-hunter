#include <gen/sapphiremusic.h>

namespace userapp {
const char* SapphireMusicStrings[] {
@foreach name in musicnames:
	"@name@",
@
	"Random song",
};
}  // namespace userapp
