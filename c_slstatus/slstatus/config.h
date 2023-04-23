/* See LICENSE file for copyright and license details. */

/* interval between updates (in ms) */
const unsigned int interval = 1000;
// 每隔 2s 更新一次
static const char unknown_str[] = "n/a";
// 如果某个值不存在，就显示 n/a（默认），比如断网时 IP 地址就会显示成 n/a
#define MAXLEN 2048
// 状态栏最大字符数量 MAXLEN，如果超过了它，第 MAXLEN 个字符及其右边的字符会被丢弃。
// 下面的注释介绍了 slstatus 的各种函数，每个函数负责显示一项信息。

/*
 * 函数                 描述                                 参数 (示例)
 *
 * battery_perc        电量（百分比形式）                  电池名称 (BAT0)
 *                                                         在 OpenBSD/FreeBSD 上无需参数
 * battery_state       电池的状态：是不是正在充电          电池名称 (BAT0)
 *                                                         在 OpenBSD/FreeBSD 上无需参数
 * battery_remaining   剩下的电量还能撑多久                电池名称 (BAT0)
 *                                                         在 OpenBSD/FreeBSD 上无需参数
 * cpu_perc            CPU 的利用率（百分比形式）          无
 * cpu_freq            CPU 的频率（单位是 MHz）            无
 * datetime            日期和时间                          格式化字符串 (%F %T)
 * disk_free           硬盘还剩多少（单位是 GB）           硬盘的挂载点 (/)
 * disk_perc           硬盘已经使用了多少 （百分比形式）   硬盘的挂载点 (/)
 * disk_total          硬盘总量（单位是 GB）               硬盘的挂载点 (/)
 * disk_used           硬盘已经使用了多少（单位是 GB）     硬盘的挂载点 (/)
 * entropy             系统的可用熵值，该值越大系统越安全  无
 * gid                 当前用户的 GID                      无
 * hostname            主机名                              无
 * ipv4                IPv4 的地址                         网卡名字 (eth0)
 * ipv6                IPv6 的地址                         网卡名字 (eth0)
 * kernel_release      内核版本号                          无
 * keyboard_indicators 显示大写锁定和数字键盘是否开启      格式化字符串 (c?n?)

 * 参数由字母 c 和字母 n 构成，大小写都可以。c 表示大写锁定，n 表示数字键盘。
 * c 和 n 后面都可以附加一个问号"?"。
 * 如果加了问号 "?"，只有锁定开启时才显示字母。比如开启大写锁定时显示 c，关闭时不显示。
 * 如果没加问号"?"，锁定开启时显示大写字母，关闭时显示小写字母。

 * keymap              键盘布局                            无
 * load_avg            系统负载（共 3 个值，分别是 1 分钟、5 分钟和 15 分钟的平均负载）无
 * netspeed_rx         下行网速                            网卡名字 (wlan0)
 * netspeed_tx         上传网速                            网卡名字 (wlan0)
 * num_files           指定文件夹的文件数量                该文件夹的路径 (/home/foo/Inbox/cur)
 * ram_free            内存还剩多少（单位是 GB）           无
 * ram_perc            内存已经使用了多少（百分比形式）    无
 * ram_total           内存总量（单位是 GB)                无
 * ram_used            内存已经使用了多少（单位是 GB）     无
 * run_command         执行一个自定义的 shell 命令         具体的 shell 命令 (echo foo)
 * separator           各信息之间的分隔符                  无
 * swap_free           交换空间还剩多少（单位是 GB）       无
 * swap_perc           交换空间已经使用了多少（百分比形式）无
 * swap_total          交换空间总量（单位是 GB）           无
 * swap_used           交换空间已经使用了多少（单位是 GB） 无
 * temp                温度（单位是摄氏度）                传感器文件的路径 (/sys/class/thermal/...)
 *                                                         在 OpenBSD 上无需参数
 *                                                         在 FreeBSD 上需要热区 (thermal zone) 作参数 (tz0, tz1, etc.)
 * uid                 当前用户的 UID                      无
 * uptime              系统已经开机运行了多久              无
 * username            当前用户的用户名                    无
 * vol_perc            OSS/ALSA 音量（百分比形式）         mixer 文件的路径 (/dev/mixer)
 *                                                         在 OpenBSD 上无需参数
 * wifi_perc           WiFi 信号强度（百分比形式）         网卡名字 (wlan0)
 * wifi_essid          WiFi 的 ESSID                       网卡名字 (wlan0)
 */
static const struct arg args[] = {// 选择上面的函数，在下面添加你想让状态栏显示的信息
    // 函数          格式           参数
    // { ipv4,         " %s",         "wlan0" },
    // %s 代表 ipv4 的值；参数需要用双引号引起来，不能是单引号，
    // 因为 C 语言里单引号内的是字符常量，双引号内的是字符串常量，两者不一样。
    // { separator,    "|",           NULL },
    // 用 "|" 间隔两项信息；参数 NULL 表示空，就是没有参数
    // { battery_perc, "Battery %s%%","BAT0" },
    // 在 C 语言里 "%" 需要转义，用 "%%" 来表示
    // { battery_state,"%s",          "BAT0" },
    // { separator,    "|",           NULL },
    // { disk_free,    "Disk %s",     "/" },
    // { separator,    "|",           NULL },
    // { separator,    "|",           NULL },
    // { ram_free,     "Memory %s",   NULL },
    // { cpu_perc,     "CPU %s%%",    NULL },
    // { separator,    "|",           NULL },
    { datetime,     "📆 %s",         "%Y年%m月%d日 星期%u" },  // %H:%M%S

    { separator,    " | ",           NULL },

    { datetime,     "⏰ %s",         "%T" },  // %H:%M%S

    { separator,    " | ",           NULL },

    { vol_perc,     "🔊 %s",         "/dev/mixer" },
    // { run_command,  "🔊 %s",         "pamixer --get-volume-human" },
};
