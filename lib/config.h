/**
 * SECTION:config
 * @short_description: Compiletime Information
 * @title: Config
 * @section_id:
 * @stability: Stable
 * @include: glyr/config.h
 *
 * Misc. version checking macros and other defines
 * that may vary between builds.
 */

#define GLYR_VERSION_MAJOR "1"
#define GLYR_VERSION_MINOR "0"
#define GLYR_VERSION_MICRO "0"

#define GLYR_VERSION_MAJOR_INT 1
#define GLYR_VERSION_MINOR_INT 0
#define GLYR_VERSION_MICRO_INT 0

/**
 * GLYR_CHECK_VERSION:
 * @X: Macro version.
 * @Y: Mino version.
 * @Z: Micro version,
 *
 * Version to check glyr's version.
 * Example:
 * <informalexample>
 * <programlisting>
 * #if GLYR_CHECK_VERSION(0,8,7)
 *      puts("Version is at least 0.87!");
 * #endif
 * </programlisting>
 * </informalexample>
*/
#define GLYR_CHECK_VERSION(X,Y,Z) (X <= GLYR_VERSION_MAJOR_INT || Y <= GLYR_VERSION_MINOR_INT || Z <= GLYR_VERSION_MICRO_INT)

#define GLYR_VERSION_NAME  "Bloody Boar"
#define GLYR_DEBUG         TRUE

/* Message output */
#define GLYR_OUTPUT stderr
