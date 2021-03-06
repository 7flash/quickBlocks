/*-------------------------------------------------------------------------
 * This source code is confidential proprietary information which is
 * Copyright (c) 2017 by Great Hill Corporation.
 * All Rights Reserved
 *
 * The LICENSE at the root of this repo details your rights (if any)
 *------------------------------------------------------------------------*/
#include <ncurses.h>
#ifdef USE_BSD
#include <bsd/stdlib.h>
#endif
#include "etherlib.h"
#include "parselib.h"
#include "processing.h"
#include "debug.h"

//EXISTING_CODE
uint64_t getLatestBloomFromCache(void) {
    uint64_t cache = 0, client = 0;
    getLatestBlocks(cache, client, NULL);
    return cache;
}
//EXISTING_CODE

//-----------------------------------------------------------------------
int main(int argc, const char *argv[]) {

    parselib_init(myQuitHandler);
    if (argc < 2)
        verbose = true;

    blknum_t clientHeight;
    uint64_t cacheHeight;
    getLatestBlocks(cacheHeight, clientHeight);

    // Parse command line, allowing for command files
    CVisitor visitor;
    if (!visitor.opts.prepareArguments(argc, argv)) {
        return 0;
    }

    // while (!visitor.opts.commandList.empty())
    {
        SFString command = nextTokenClear(visitor.opts.commandList, '\n');
        if (!visitor.opts.parseArguments(command)) {
            return 0;
        }

        if (!fileExists("./config.toml")) {
            cerr << "The config.toml file was not found. Are you in the right folder? Quitting...\n";
            cerr.flush();
            return 0;
        }

        CToml toml("./config.toml");
        visitor.loadWatches(toml);

        const char* defaultFormat = "{ \"date\": \"[{DATE}]\", \"from\": \"[{FROM}]\", \"to\": \"[{TO}]\", \"value\": \"[{VALUE}]\" }";
        visitor.screenFmt          = cleanFmt(toml.getConfigStr("formats", "screen_fmt",  defaultFormat));
        visitor.opts.accounting_on = toml.getConfigBool("display", "accounting", false) || visitor.opts.accounting_on;
        visitor.opts.logs_on       = toml.getConfigBool("display", "logs", false) || visitor.opts.logs_on;
        visitor.opts.trace_on      = toml.getConfigBool("display", "trace", false) || visitor.opts.trace_on;
        visitor.opts.parse_on      = toml.getConfigBool("display", "parse", false) || visitor.opts.parse_on;
        visitor.opts.bloom_on      = toml.getConfigBool("display", "bloom", false) || visitor.opts.bloom_on;
        visitor.opts.debugger_on   = toml.getConfigBool("display", "debug", false) || visitor.opts.debugger_on;
        visitor.opts.single_on     = toml.getConfigBool("display", "single", false) || visitor.opts.single_on;
        visitor.opts.kBlock        = visitor.opts.kBlock;
        visitor.opts.mode          = visitor.opts.mode;
        visitor.opts.monitorName   = toml.getConfigStr("settings", "name", "") + " ";

        // Showing the cache file (if told to...)
        SFString cacheFileName = "./cache/" + visitor.watches[0].address + ".acct.bin";
        if (fileExists(cacheFileName + ".lck")) {
            cout << usageStr("The cache lock file is present. The program is either already running or it did not end cleanly the\n"
                            "\tlast time it ran. Quit the already running program or, if it is not running, remove the lock\n"
                            "\tfile: " + cacheFileName + ".lck'. Quitting...")
                    .Substitute("\n", "\r\n");
            cout.flush();cerr.flush();//getchar();
            return 0;
        }

        // Figure out which block to start on. Use earliest block from the watches. Note that
        // 'displayFromCache' may modify this to lastest visited block
        bool upToDate = false;
        uint64_t blockNum = visitor.blockStats.minWatchBlock-1;
        if (visitor.opts.kBlock) {  // we're not starting at the beginning
            blockNum = visitor.opts.kBlock;
            for (uint32_t i = 0 ; i < visitor.watches.getCount() ; i++) {
                visitor.watches[i].qbis.endBal = getBalance(visitor.watches[i].address, blockNum, false);
            }
            upToDate = true;
        }

        if (visitor.opts.debugger_on) {
            removeFile("./cache/debug");
            initscr();
            raw();
            keypad(stdscr, true);
            noecho();
            refresh();
            atexit(myOnExitHandler);
            cout << "Starting balances:\r\n"; cout.flush();
        }

        // Display the cache (if the user tells us to...)
        if (!visitor.opts.debugger_on && !verbose) verbose = 1;
        if (visitor.opts.mode.Contains("showCache")) {
            // TODO(tjayrush): allow for early quiting from debugger--trouble--with this on, and no cache, it
            // immediately quits because displayFromCache returns 'false' for more than one reason
            if (!displayFromCache(cacheFileName, blockNum, &visitor))
                visitor.opts.mode = ""; // do not continue
            upToDate = true;
        }

        // Freshening the cache (if the user tells us to...)
        if (visitor.opts.mode.Contains("freshen")) {

            visitor.checkForImport();

            uint64_t lastVisit  = toLongU(asciiFileToString("./cache/lastBlock.txt"));
            blockNum = max(blockNum, lastVisit + 1);

            visitor.blockStats.lastBlock  = min(cacheHeight, visitor.blockStats.maxWatchBlock);
            visitor.blockStats.firstBlock = min(blockNum,    visitor.blockStats.lastBlock);
            ASSERT(visitor.blockStats.firstBlock <= visitor.blockStats.lastBlock);
            visitor.blockStats.nBlocks    = visitor.blockStats.lastBlock - visitor.blockStats.firstBlock;
            cerr << "Freshening " << visitor.opts.monitorName
                    << "from " << visitor.blockStats.firstBlock
                    << " to " << visitor.blockStats.lastBlock
                    << " inclusive (" << visitor.blockStats.nBlocks << " blocks)\r\n";
            cerr.flush();

            if (!upToDate) {  // we're not starting at the beginning
                for (uint32_t i = 0 ; i < visitor.watches.getCount() ; i++) {
                    visitor.watches[i].qbis.endBal = getBalance(visitor.watches[i].address, visitor.blockStats.firstBlock, false);
                }
            }

            // The cache may have been opened for reading during displayFromCache, so we
            // close it here, so we can open it back up as append only
            visitor.cache.resetForWriting();
            if (visitor.cache.Lock(cacheFileName, "a+", LOCK_WAIT)) {
//cout << "firstBlock: " << visitor.blockStats.firstBlock << " nBlocks: " << visitor.blockStats.nBlocks << "\n";
//cout.flush();
                forEveryBloomFile(updateCacheUsingBlooms, &visitor, visitor.blockStats.firstBlock, visitor.blockStats.nBlocks);
                visitor.cache.Release();
            }

            if (visitor.transStats.nFreshened) {
                SFTime dt = dateFromTimeStamp(visitor.blockStats.prevBlock.timestamp);
                progressBar(visitor.blockStats.nBlocks, visitor.blockStats.nBlocks, visitor.opts.monitorName+"|"+dt.Format(FMT_JSON) + " (" + asStringU(cacheHeight) + ")");
                cout << "\r\n";
            }
        }

        SFTime now = Now();
        cout << getprogname() << ": " << now.Format(FMT_JSON) << ": "
                << "{ "
                << cYellow << visitor.transStats.nDisplayed    << cOff << " displayed from cache; "
                << cYellow << visitor.transStats.nFreshened    << cOff << " written to cache; "
                << cYellow << visitor.transStats.nAccountedFor << cOff << " accounted for"
                << " }\r\n";

        if (visitor.opts.debugger_on) {
            // If we were debugging and we did nothing, let the user know
            if ((visitor.transStats.nDisplayed + visitor.transStats.nFreshened + visitor.transStats.nAccountedFor) == 0) {
                cout << "Nothing to do. Hit enter to quit...";
                cout.flush();
                getchar();
            }
        }
    }

    if (visitor.opts.debugger_on) {
        CBlock block;
        getBlock(block, cacheHeight);
        visitor.enterDebugger(block);
    }

    return 0;
}

//-----------------------------------------------------------------------
void myQuitHandler(int s) {
    if (!isendwin())
        endwin();
    if (s != 1)
        cout << "Caught signal " << s << "\n";
    SFString list = manageRemoveList();
    while (!list.empty()) {
        SFString file = nextTokenClear(list, '|');
        cout << "Removing file: " << file << "\n"; cout.flush();
        removeFile(file);
    }
    removeFile("./cache/debug");
    exit(1);
}

//EXISTING_CODE
//EXISTING_CODE
