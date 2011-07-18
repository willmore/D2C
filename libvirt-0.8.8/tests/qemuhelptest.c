#include <config.h>

#ifdef WITH_QEMU

# include <stdio.h>
# include <stdlib.h>

# include "testutils.h"
# include "qemu/qemu_capabilities.h"
# include "memory.h"

# define MAX_HELP_OUTPUT_SIZE 1024*64

struct testInfo {
    const char *name;
    unsigned long long flags;
    unsigned int version;
    unsigned int is_kvm;
    unsigned int kvm_version;
};

static char *progname;
static char *abs_srcdir;

static void printMismatchedFlags(unsigned long long got,
                                 unsigned long long expect)
{
    int i;

    for (i = 0 ; i < (sizeof(got)*CHAR_BIT) ; i++) {
        unsigned long long gotFlag = (got & (1LL << i));
        unsigned long long expectFlag = (expect & (1LL << i));
        if (gotFlag && !expectFlag)
            fprintf(stderr, "Extra flag %i\n", i);
        if (!gotFlag && expectFlag)
            fprintf(stderr, "Missing flag %i\n", i);
    }
}

static int testHelpStrParsing(const void *data)
{
    const struct testInfo *info = data;
    char *path = NULL;
    char helpStr[MAX_HELP_OUTPUT_SIZE];
    char *help = &(helpStr[0]);
    unsigned int version, is_kvm, kvm_version;
    unsigned long long flags;
    int ret = -1;

    if (virAsprintf(&path, "%s/qemuhelpdata/%s", abs_srcdir, info->name) < 0)
        return -1;

    if (virtTestLoadFile(path, &help, MAX_HELP_OUTPUT_SIZE) < 0)
        goto cleanup;

    if (qemuCapsParseHelpStr("QEMU", help, &flags,
                             &version, &is_kvm, &kvm_version) == -1)
        goto cleanup;

    if (info->flags & QEMUD_CMD_FLAG_DEVICE) {
        VIR_FREE(path);
        if (virAsprintf(&path, "%s/qemuhelpdata/%s-device", abs_srcdir,
                        info->name) < 0)
            goto cleanup;

        if (virtTestLoadFile(path, &help, MAX_HELP_OUTPUT_SIZE) < 0)
            goto cleanup;

        if (qemuCapsParseDeviceStr(help, &flags) < 0)
            goto cleanup;
    }

    if (flags != info->flags) {
        fprintf(stderr,
                "Computed flags do not match: got 0x%llx, expected 0x%llx\n",
                flags, info->flags);

        if (getenv("VIR_TEST_DEBUG"))
            printMismatchedFlags(flags, info->flags);

        goto cleanup;
    }

    if (version != info->version) {
        fprintf(stderr, "Parsed versions do not match: got %u, expected %u\n",
                version, info->version);
        goto cleanup;
    }

    if (is_kvm != info->is_kvm) {
        fprintf(stderr,
                "Parsed is_kvm flag does not match: got %u, expected %u\n",
                is_kvm, info->is_kvm);
        goto cleanup;
    }

    if (kvm_version != info->kvm_version) {
        fprintf(stderr,
                "Parsed KVM versions do not match: got %u, expected %u\n",
                kvm_version, info->kvm_version);
        goto cleanup;
    }

    ret = 0;
cleanup:
    VIR_FREE(path);
    return ret;
}

static int
mymain(int argc, char **argv)
{
    int ret = 0;
    char cwd[PATH_MAX];

    progname = argv[0];

    if (argc > 1) {
        fprintf(stderr, "Usage: %s\n", progname);
        return (EXIT_FAILURE);
    }

    abs_srcdir = getenv("abs_srcdir");
    if (!abs_srcdir)
        abs_srcdir = getcwd(cwd, sizeof(cwd));

# define DO_TEST(name, flags, version, is_kvm, kvm_version)                          \
    do {                                                                            \
        const struct testInfo info = { name, flags, version, is_kvm, kvm_version }; \
        if (virtTestRun("QEMU Help String Parsing " name,                           \
                        1, testHelpStrParsing, &info) < 0)                          \
            ret = -1;                                                               \
    } while (0)

    DO_TEST("qemu-0.9.1",
            QEMUD_CMD_FLAG_KQEMU |
            QEMUD_CMD_FLAG_VNC_COLON |
            QEMUD_CMD_FLAG_NO_REBOOT |
            QEMUD_CMD_FLAG_DRIVE |
            QEMUD_CMD_FLAG_NAME,
            9001,  0,  0);
    DO_TEST("kvm-74",
            QEMUD_CMD_FLAG_VNC_COLON |
            QEMUD_CMD_FLAG_NO_REBOOT |
            QEMUD_CMD_FLAG_DRIVE |
            QEMUD_CMD_FLAG_DRIVE_BOOT |
            QEMUD_CMD_FLAG_NAME |
            QEMUD_CMD_FLAG_VNET_HDR |
            QEMUD_CMD_FLAG_MIGRATE_KVM_STDIO |
            QEMUD_CMD_FLAG_KVM |
            QEMUD_CMD_FLAG_DRIVE_FORMAT |
            QEMUD_CMD_FLAG_MEM_PATH |
            QEMUD_CMD_FLAG_TDF,
            9001,  1, 74);
    DO_TEST("kvm-83-rhel56",
            QEMUD_CMD_FLAG_VNC_COLON |
            QEMUD_CMD_FLAG_NO_REBOOT |
            QEMUD_CMD_FLAG_DRIVE |
            QEMUD_CMD_FLAG_DRIVE_BOOT |
            QEMUD_CMD_FLAG_NAME |
            QEMUD_CMD_FLAG_UUID |
            QEMUD_CMD_FLAG_VNET_HDR |
            QEMUD_CMD_FLAG_MIGRATE_QEMU_TCP |
            QEMUD_CMD_FLAG_MIGRATE_QEMU_EXEC |
            QEMUD_CMD_FLAG_DRIVE_CACHE_V2 |
            QEMUD_CMD_FLAG_KVM |
            QEMUD_CMD_FLAG_DRIVE_FORMAT |
            QEMUD_CMD_FLAG_DRIVE_SERIAL |
            QEMUD_CMD_FLAG_VGA |
            QEMUD_CMD_FLAG_PCIDEVICE |
            QEMUD_CMD_FLAG_MEM_PATH |
            QEMUD_CMD_FLAG_BALLOON |
            QEMUD_CMD_FLAG_RTC_TD_HACK |
            QEMUD_CMD_FLAG_NO_HPET |
            QEMUD_CMD_FLAG_NO_KVM_PIT |
            QEMUD_CMD_FLAG_TDF |
            QEMUD_CMD_FLAG_DRIVE_READONLY |
            QEMUD_CMD_FLAG_SMBIOS_TYPE |
            QEMUD_CMD_FLAG_SPICE,
            9001, 1,  83);
    DO_TEST("qemu-0.10.5",
            QEMUD_CMD_FLAG_KQEMU |
            QEMUD_CMD_FLAG_VNC_COLON |
            QEMUD_CMD_FLAG_NO_REBOOT |
            QEMUD_CMD_FLAG_DRIVE |
            QEMUD_CMD_FLAG_NAME |
            QEMUD_CMD_FLAG_UUID |
            QEMUD_CMD_FLAG_MIGRATE_QEMU_TCP |
            QEMUD_CMD_FLAG_MIGRATE_QEMU_EXEC |
            QEMUD_CMD_FLAG_DRIVE_CACHE_V2 |
            QEMUD_CMD_FLAG_DRIVE_FORMAT |
            QEMUD_CMD_FLAG_DRIVE_SERIAL |
            QEMUD_CMD_FLAG_VGA |
            QEMUD_CMD_FLAG_0_10 |
            QEMUD_CMD_FLAG_ENABLE_KVM |
            QEMUD_CMD_FLAG_SDL |
            QEMUD_CMD_FLAG_RTC_TD_HACK |
            QEMUD_CMD_FLAG_NO_HPET |
            QEMUD_CMD_FLAG_VGA_NONE,
            10005, 0,  0);
    DO_TEST("qemu-kvm-0.10.5",
            QEMUD_CMD_FLAG_VNC_COLON |
            QEMUD_CMD_FLAG_NO_REBOOT |
            QEMUD_CMD_FLAG_DRIVE |
            QEMUD_CMD_FLAG_DRIVE_BOOT |
            QEMUD_CMD_FLAG_NAME |
            QEMUD_CMD_FLAG_UUID |
            QEMUD_CMD_FLAG_VNET_HDR |
            QEMUD_CMD_FLAG_MIGRATE_QEMU_TCP |
            QEMUD_CMD_FLAG_MIGRATE_QEMU_EXEC |
            QEMUD_CMD_FLAG_DRIVE_CACHE_V2 |
            QEMUD_CMD_FLAG_KVM |
            QEMUD_CMD_FLAG_DRIVE_FORMAT |
            QEMUD_CMD_FLAG_DRIVE_SERIAL |
            QEMUD_CMD_FLAG_VGA |
            QEMUD_CMD_FLAG_0_10 |
            QEMUD_CMD_FLAG_PCIDEVICE |
            QEMUD_CMD_FLAG_MEM_PATH |
            QEMUD_CMD_FLAG_SDL |
            QEMUD_CMD_FLAG_RTC_TD_HACK |
            QEMUD_CMD_FLAG_NO_HPET |
            QEMUD_CMD_FLAG_NO_KVM_PIT |
            QEMUD_CMD_FLAG_TDF |
            QEMUD_CMD_FLAG_NESTING |
            QEMUD_CMD_FLAG_VGA_NONE,
            10005, 1,  0);
    DO_TEST("kvm-86",
            QEMUD_CMD_FLAG_VNC_COLON |
            QEMUD_CMD_FLAG_NO_REBOOT |
            QEMUD_CMD_FLAG_DRIVE |
            QEMUD_CMD_FLAG_DRIVE_BOOT |
            QEMUD_CMD_FLAG_NAME |
            QEMUD_CMD_FLAG_UUID |
            QEMUD_CMD_FLAG_VNET_HDR |
            QEMUD_CMD_FLAG_MIGRATE_QEMU_TCP |
            QEMUD_CMD_FLAG_MIGRATE_QEMU_EXEC |
            QEMUD_CMD_FLAG_DRIVE_CACHE_V2 |
            QEMUD_CMD_FLAG_KVM |
            QEMUD_CMD_FLAG_DRIVE_FORMAT |
            QEMUD_CMD_FLAG_DRIVE_SERIAL |
            QEMUD_CMD_FLAG_VGA |
            QEMUD_CMD_FLAG_0_10 |
            QEMUD_CMD_FLAG_PCIDEVICE |
            QEMUD_CMD_FLAG_SDL |
            QEMUD_CMD_FLAG_RTC_TD_HACK |
            QEMUD_CMD_FLAG_NO_HPET |
            QEMUD_CMD_FLAG_NO_KVM_PIT |
            QEMUD_CMD_FLAG_TDF |
            QEMUD_CMD_FLAG_NESTING |
            QEMUD_CMD_FLAG_SMBIOS_TYPE |
            QEMUD_CMD_FLAG_VGA_NONE,
            10050, 1,  0);
    DO_TEST("qemu-kvm-0.11.0-rc2",
            QEMUD_CMD_FLAG_VNC_COLON |
            QEMUD_CMD_FLAG_NO_REBOOT |
            QEMUD_CMD_FLAG_DRIVE |
            QEMUD_CMD_FLAG_DRIVE_BOOT |
            QEMUD_CMD_FLAG_NAME |
            QEMUD_CMD_FLAG_UUID |
            QEMUD_CMD_FLAG_VNET_HDR |
            QEMUD_CMD_FLAG_MIGRATE_QEMU_TCP |
            QEMUD_CMD_FLAG_MIGRATE_QEMU_EXEC |
            QEMUD_CMD_FLAG_DRIVE_CACHE_V2 |
            QEMUD_CMD_FLAG_KVM |
            QEMUD_CMD_FLAG_DRIVE_FORMAT |
            QEMUD_CMD_FLAG_DRIVE_SERIAL |
            QEMUD_CMD_FLAG_VGA |
            QEMUD_CMD_FLAG_0_10 |
            QEMUD_CMD_FLAG_PCIDEVICE |
            QEMUD_CMD_FLAG_MEM_PATH |
            QEMUD_CMD_FLAG_ENABLE_KVM |
            QEMUD_CMD_FLAG_BALLOON |
            QEMUD_CMD_FLAG_SDL |
            QEMUD_CMD_FLAG_RTC_TD_HACK |
            QEMUD_CMD_FLAG_NO_HPET |
            QEMUD_CMD_FLAG_NO_KVM_PIT |
            QEMUD_CMD_FLAG_TDF |
            QEMUD_CMD_FLAG_BOOT_MENU |
            QEMUD_CMD_FLAG_NESTING |
            QEMUD_CMD_FLAG_NAME_PROCESS |
            QEMUD_CMD_FLAG_SMBIOS_TYPE |
            QEMUD_CMD_FLAG_VGA_NONE,
            10092, 1,  0);
    DO_TEST("qemu-0.12.1",
            QEMUD_CMD_FLAG_VNC_COLON |
            QEMUD_CMD_FLAG_NO_REBOOT |
            QEMUD_CMD_FLAG_DRIVE |
            QEMUD_CMD_FLAG_NAME |
            QEMUD_CMD_FLAG_UUID |
            QEMUD_CMD_FLAG_MIGRATE_QEMU_TCP |
            QEMUD_CMD_FLAG_MIGRATE_QEMU_EXEC |
            QEMUD_CMD_FLAG_DRIVE_CACHE_V2 |
            QEMUD_CMD_FLAG_DRIVE_FORMAT |
            QEMUD_CMD_FLAG_DRIVE_SERIAL |
            QEMUD_CMD_FLAG_DRIVE_READONLY |
            QEMUD_CMD_FLAG_VGA |
            QEMUD_CMD_FLAG_0_10 |
            QEMUD_CMD_FLAG_ENABLE_KVM |
            QEMUD_CMD_FLAG_SDL |
            QEMUD_CMD_FLAG_XEN_DOMID |
            QEMUD_CMD_FLAG_MIGRATE_QEMU_UNIX |
            QEMUD_CMD_FLAG_CHARDEV |
            QEMUD_CMD_FLAG_BALLOON |
            QEMUD_CMD_FLAG_DEVICE |
            QEMUD_CMD_FLAG_SMP_TOPOLOGY |
            QEMUD_CMD_FLAG_RTC |
            QEMUD_CMD_FLAG_NO_HPET |
            QEMUD_CMD_FLAG_BOOT_MENU |
            QEMUD_CMD_FLAG_NAME_PROCESS |
            QEMUD_CMD_FLAG_SMBIOS_TYPE |
            QEMUD_CMD_FLAG_VGA_NONE |
            QEMUD_CMD_FLAG_MIGRATE_QEMU_FD |
            QEMUD_CMD_FLAG_DRIVE_AIO,
            12001, 0,  0);
    DO_TEST("qemu-kvm-0.12.1.2-rhel60",
            QEMUD_CMD_FLAG_VNC_COLON |
            QEMUD_CMD_FLAG_NO_REBOOT |
            QEMUD_CMD_FLAG_DRIVE |
            QEMUD_CMD_FLAG_DRIVE_BOOT |
            QEMUD_CMD_FLAG_NAME |
            QEMUD_CMD_FLAG_UUID |
            QEMUD_CMD_FLAG_VNET_HDR |
            QEMUD_CMD_FLAG_MIGRATE_QEMU_TCP |
            QEMUD_CMD_FLAG_MIGRATE_QEMU_EXEC |
            QEMUD_CMD_FLAG_DRIVE_CACHE_V2 |
            QEMUD_CMD_FLAG_KVM |
            QEMUD_CMD_FLAG_DRIVE_FORMAT |
            QEMUD_CMD_FLAG_DRIVE_SERIAL |
            QEMUD_CMD_FLAG_DRIVE_READONLY |
            QEMUD_CMD_FLAG_VGA |
            QEMUD_CMD_FLAG_0_10 |
            QEMUD_CMD_FLAG_PCIDEVICE |
            QEMUD_CMD_FLAG_MEM_PATH |
            QEMUD_CMD_FLAG_MIGRATE_QEMU_UNIX |
            QEMUD_CMD_FLAG_CHARDEV |
            QEMUD_CMD_FLAG_ENABLE_KVM |
            QEMUD_CMD_FLAG_BALLOON |
            QEMUD_CMD_FLAG_DEVICE |
            QEMUD_CMD_FLAG_SMP_TOPOLOGY |
            QEMUD_CMD_FLAG_RTC |
            QEMUD_CMD_FLAG_VNET_HOST |
            QEMUD_CMD_FLAG_NO_KVM_PIT |
            QEMUD_CMD_FLAG_TDF |
            QEMUD_CMD_FLAG_PCI_CONFIGFD |
            QEMUD_CMD_FLAG_NODEFCONFIG |
            QEMUD_CMD_FLAG_BOOT_MENU |
            QEMUD_CMD_FLAG_NESTING |
            QEMUD_CMD_FLAG_NAME_PROCESS |
            QEMUD_CMD_FLAG_SMBIOS_TYPE |
            QEMUD_CMD_FLAG_VGA_QXL |
            QEMUD_CMD_FLAG_SPICE |
            QEMUD_CMD_FLAG_VGA_NONE |
            QEMUD_CMD_FLAG_MIGRATE_QEMU_FD |
            QEMUD_CMD_FLAG_DRIVE_AIO |
            QEMUD_CMD_FLAG_DEVICE_SPICEVMC,
            12001, 1,  0);
    DO_TEST("qemu-kvm-0.12.3",
            QEMUD_CMD_FLAG_VNC_COLON |
            QEMUD_CMD_FLAG_NO_REBOOT |
            QEMUD_CMD_FLAG_DRIVE |
            QEMUD_CMD_FLAG_DRIVE_BOOT |
            QEMUD_CMD_FLAG_NAME |
            QEMUD_CMD_FLAG_UUID |
            QEMUD_CMD_FLAG_VNET_HDR |
            QEMUD_CMD_FLAG_MIGRATE_QEMU_TCP |
            QEMUD_CMD_FLAG_MIGRATE_QEMU_EXEC |
            QEMUD_CMD_FLAG_DRIVE_CACHE_V2 |
            QEMUD_CMD_FLAG_KVM |
            QEMUD_CMD_FLAG_DRIVE_FORMAT |
            QEMUD_CMD_FLAG_DRIVE_SERIAL |
            QEMUD_CMD_FLAG_DRIVE_READONLY |
            QEMUD_CMD_FLAG_VGA |
            QEMUD_CMD_FLAG_0_10 |
            QEMUD_CMD_FLAG_PCIDEVICE |
            QEMUD_CMD_FLAG_MEM_PATH |
            QEMUD_CMD_FLAG_SDL |
            QEMUD_CMD_FLAG_MIGRATE_QEMU_UNIX |
            QEMUD_CMD_FLAG_CHARDEV |
            QEMUD_CMD_FLAG_BALLOON |
            QEMUD_CMD_FLAG_DEVICE |
            QEMUD_CMD_FLAG_SMP_TOPOLOGY |
            QEMUD_CMD_FLAG_RTC |
            QEMUD_CMD_FLAG_VNET_HOST |
            QEMUD_CMD_FLAG_NO_HPET |
            QEMUD_CMD_FLAG_NO_KVM_PIT |
            QEMUD_CMD_FLAG_TDF |
            QEMUD_CMD_FLAG_BOOT_MENU |
            QEMUD_CMD_FLAG_NESTING |
            QEMUD_CMD_FLAG_NAME_PROCESS |
            QEMUD_CMD_FLAG_SMBIOS_TYPE |
            QEMUD_CMD_FLAG_VGA_NONE |
            QEMUD_CMD_FLAG_MIGRATE_QEMU_FD |
            QEMUD_CMD_FLAG_DRIVE_AIO,
            12003, 1,  0);
    DO_TEST("qemu-kvm-0.13.0",
            QEMUD_CMD_FLAG_VNC_COLON |
            QEMUD_CMD_FLAG_NO_REBOOT |
            QEMUD_CMD_FLAG_DRIVE |
            QEMUD_CMD_FLAG_DRIVE_BOOT |
            QEMUD_CMD_FLAG_NAME |
            QEMUD_CMD_FLAG_UUID |
            QEMUD_CMD_FLAG_VNET_HDR |
            QEMUD_CMD_FLAG_MIGRATE_QEMU_TCP |
            QEMUD_CMD_FLAG_MIGRATE_QEMU_EXEC |
            QEMUD_CMD_FLAG_DRIVE_CACHE_V2 |
            QEMUD_CMD_FLAG_KVM |
            QEMUD_CMD_FLAG_DRIVE_FORMAT |
            QEMUD_CMD_FLAG_DRIVE_SERIAL |
            QEMUD_CMD_FLAG_XEN_DOMID |
            QEMUD_CMD_FLAG_DRIVE_READONLY |
            QEMUD_CMD_FLAG_VGA |
            QEMUD_CMD_FLAG_0_10 |
            QEMUD_CMD_FLAG_PCIDEVICE |
            QEMUD_CMD_FLAG_MEM_PATH |
            QEMUD_CMD_FLAG_SDL |
            QEMUD_CMD_FLAG_MIGRATE_QEMU_UNIX |
            QEMUD_CMD_FLAG_CHARDEV |
            QEMUD_CMD_FLAG_ENABLE_KVM |
            QEMUD_CMD_FLAG_MONITOR_JSON |
            QEMUD_CMD_FLAG_BALLOON |
            QEMUD_CMD_FLAG_DEVICE |
            QEMUD_CMD_FLAG_SMP_TOPOLOGY |
            QEMUD_CMD_FLAG_NETDEV |
            QEMUD_CMD_FLAG_RTC |
            QEMUD_CMD_FLAG_VNET_HOST |
            QEMUD_CMD_FLAG_NO_HPET |
            QEMUD_CMD_FLAG_NO_KVM_PIT |
            QEMUD_CMD_FLAG_TDF |
            QEMUD_CMD_FLAG_PCI_CONFIGFD |
            QEMUD_CMD_FLAG_NODEFCONFIG |
            QEMUD_CMD_FLAG_BOOT_MENU |
            QEMUD_CMD_FLAG_FSDEV |
            QEMUD_CMD_FLAG_NESTING |
            QEMUD_CMD_FLAG_NAME_PROCESS |
            QEMUD_CMD_FLAG_SMBIOS_TYPE |
            QEMUD_CMD_FLAG_SPICE |
            QEMUD_CMD_FLAG_VGA_NONE |
            QEMUD_CMD_FLAG_MIGRATE_QEMU_FD |
            QEMUD_CMD_FLAG_DRIVE_AIO |
            QEMUD_CMD_FLAG_DEVICE_SPICEVMC,
            13000, 1,  0);
    DO_TEST("qemu-kvm-0.12.1.2-rhel61",
            QEMUD_CMD_FLAG_VNC_COLON |
            QEMUD_CMD_FLAG_NO_REBOOT |
            QEMUD_CMD_FLAG_DRIVE |
            QEMUD_CMD_FLAG_NAME |
            QEMUD_CMD_FLAG_UUID |
            QEMUD_CMD_FLAG_VNET_HDR |
            QEMUD_CMD_FLAG_MIGRATE_QEMU_TCP |
            QEMUD_CMD_FLAG_MIGRATE_QEMU_EXEC |
            QEMUD_CMD_FLAG_DRIVE_CACHE_V2 |
            QEMUD_CMD_FLAG_KVM |
            QEMUD_CMD_FLAG_DRIVE_FORMAT |
            QEMUD_CMD_FLAG_DRIVE_SERIAL |
            QEMUD_CMD_FLAG_DRIVE_READONLY |
            QEMUD_CMD_FLAG_VGA |
            QEMUD_CMD_FLAG_0_10 |
            QEMUD_CMD_FLAG_PCIDEVICE |
            QEMUD_CMD_FLAG_MEM_PATH |
            QEMUD_CMD_FLAG_MIGRATE_QEMU_UNIX |
            QEMUD_CMD_FLAG_CHARDEV |
            QEMUD_CMD_FLAG_ENABLE_KVM |
            QEMUD_CMD_FLAG_BALLOON |
            QEMUD_CMD_FLAG_DEVICE |
            QEMUD_CMD_FLAG_SMP_TOPOLOGY |
            QEMUD_CMD_FLAG_RTC |
            QEMUD_CMD_FLAG_VNET_HOST |
            QEMUD_CMD_FLAG_NO_KVM_PIT |
            QEMUD_CMD_FLAG_TDF |
            QEMUD_CMD_FLAG_PCI_CONFIGFD |
            QEMUD_CMD_FLAG_NODEFCONFIG |
            QEMUD_CMD_FLAG_BOOT_MENU |
            QEMUD_CMD_FLAG_NESTING |
            QEMUD_CMD_FLAG_NAME_PROCESS |
            QEMUD_CMD_FLAG_SMBIOS_TYPE |
            QEMUD_CMD_FLAG_VGA_QXL |
            QEMUD_CMD_FLAG_SPICE |
            QEMUD_CMD_FLAG_VGA_NONE |
            QEMUD_CMD_FLAG_MIGRATE_QEMU_FD |
            QEMUD_CMD_FLAG_HDA_DUPLEX |
            QEMUD_CMD_FLAG_DRIVE_AIO |
            QEMUD_CMD_FLAG_CCID_PASSTHRU |
            QEMUD_CMD_FLAG_CHARDEV_SPICEVMC,
            12001, 1,  0);

    return ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

VIRT_TEST_MAIN(mymain)

#else

int main (void) { return (77); /* means 'test skipped' for automake */ }

#endif /* WITH_QEMU */
