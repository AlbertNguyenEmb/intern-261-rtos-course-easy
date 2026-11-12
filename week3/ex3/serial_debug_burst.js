/* Serial Debug Assistant SEND script.
 * Type 100 (or 1000) in its send box, then click Send ONCE.
 * Wait for the burst/logs to finish; type ? and click Send to get STATS.
 * Reset the board before each run. Disable Timing send and HEX send.
 */
(function () {
    var request = send.getString().trim();

    if (request === "?") {
        send.write("?\r", false);
        receive.write("\r\n[PC] STATS query: 2 bytes\r\n");
        return;
    }

    if (!/^[1-9][0-9]*$/.test(request)) {
        receive.write("\r\n[PC] Enter a command count (1..10000) or ?\r\n");
        return;
    }
    var count = Number(request);
    if (count > 10000) {
        receive.write("\r\n[PC] Maximum: 10000 commands per run\r\n");
        return;
    }

    var payload = "";
    for (var i = 0; i < count; i++) {
        payload += "P10#" + i + "\r";
    }

    /* One contiguous ASCII payload, no deliberate inter-command delay.
     * A request queued on the PC is not proof of delivery to the MCU. */
    send.write(payload, false);
    receive.write(
        "\r\n[PC] REQUEST count=" + count +
        " bytes=" + payload.length +
        " first=0 last=" + (count - 1) +
        " expected_rx_after_one_stats_query=" + (payload.length + 2) +
        "\r\n"
    );
})();
