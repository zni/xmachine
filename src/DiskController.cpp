#include <chrono>
#include <iostream>
#include <thread>

#include "include/DiskController.h"
#include "include/BusMessage.h"

DiskController::DiskController()
{
    m_state = DiskControllerState::BEGIN;
    m_function = DiskControllerFunction::IDLE;
    m_track = 0;
    m_sector = 0;
    m_RXCS = 0;
    m_RXDB = 0;
    m_buffer_index = 0;
    m_disk_media = NULL;
    m_bus_connection = NULL;
}

DiskController::~DiskController()
{
    if (m_disk_media != NULL) {
        fclose(m_disk_media);
    }
}

void DiskController::send(enum BusMessage t, uint32_t addr, uint16_t data)
{
    m_bus_connection->send_bus_message(this, t, addr, data);
}

void DiskController::recv(enum BusMessage t, uint32_t addr, uint16_t data)
{
    if (addr == RXCS || addr == RXDB) {
        process_bus_message(t, addr, data);
    }
}

uint16_t DiskController::bus_id()
{
    return 0000003;
}

void DiskController::set_bus(Bus *bus)
{
    m_bus_connection = bus;
}

void DiskController::insert_disk_media(char *disk)
{
    m_disk_media = fopen(disk, "r+");
}

void DiskController::execute()
{
    uint16_t go_flag;
    while (!m_bus_connection->halted()) {
        go_flag = static_cast<uint16_t>(RXCSFlag::GO) & m_RXCS;
        if (go_flag && ((m_state == DiskControllerState::BEGIN) ||
                        (m_state == DiskControllerState::DONE))) {
            printf("go(%07o)\n", m_RXCS);

            enum DiskControllerFunction function =
                static_cast<DiskControllerFunction>((static_cast<uint16_t>(RXCSFlag::FS) & m_RXCS) >> 1);

            clear_all_flags();

            switch (function) {
                case DiskControllerFunction::FILL_BUFFER:
                    m_state = DiskControllerState::FILL;
                    m_function = DiskControllerFunction::FILL_BUFFER;
                    set_transfer_flag();
                    break;
                case DiskControllerFunction::EMPTY_BUFFER:
                    m_state = DiskControllerState::EMPTY;
                    m_function = DiskControllerFunction::EMPTY_BUFFER;
                    break;
                case DiskControllerFunction::WRITE_SECTOR:
                    m_state = DiskControllerState::SECTOR;
                    m_function = DiskControllerFunction::WRITE_SECTOR;
                    set_transfer_flag();
                    break;
                case DiskControllerFunction::READ_SECTOR:
                    m_state = DiskControllerState::SECTOR;
                    m_function = DiskControllerFunction::READ_SECTOR;
                    set_transfer_flag();
                    break;
                case DiskControllerFunction::NOT_USED:
                    break;
                case DiskControllerFunction::READ_STATUS:
                    break;
                case DiskControllerFunction::WRITE_DEL:
                    break;
                case DiskControllerFunction::READ_ERR:
                    break;
                default:
                    break;
            }

        } else if ((m_state == DiskControllerState::FILL) &&
                   (m_function == DiskControllerFunction::FILL_BUFFER)) {
            printf("%s(S_FILL, %07o)\n", __FUNCTION__, m_RXCS);
            fill_buffer();

        } else if ((m_state == DiskControllerState::EMPTY) &&
                   (m_function == DiskControllerFunction::EMPTY_BUFFER)) {
            printf("%s(S_EMPTY, %07o)\n", __FUNCTION__, m_RXCS);
            empty_buffer();

        } else if (m_function == DiskControllerFunction::READ_SECTOR) {
            printf("%s(F_READ_SECTOR, %07o)\n", __FUNCTION__, m_RXCS);
            read_sector();

        } else if (m_function == DiskControllerFunction::WRITE_SECTOR) {
            printf("%s(F_WRITE_SECTOR, %07o)\n", __FUNCTION__, m_RXCS);
            write_sector();

        } else if (m_state == DiskControllerState::DONE) {
            m_state = DiskControllerState::BEGIN;
            printf("%s(S_DONE, flags: %07o)\n", __FUNCTION__, m_RXCS);
            set_done_flag();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void DiskController::dump()
{
    printf("RXCS: %07o\n", m_RXCS);
    printf("RXDB: %07o\n", m_RXDB);
    for (int i = 0; i < BUFFER_SIZE; i++) {
        printf("%03o ", m_internal_buffer[i]);
        if ((i + 1) % 16 == 0 && i != 0) {
            putchar('\n');
        }
    }
    putchar('\n');
}

void DiskController::process_bus_message(enum BusMessage t, uint32_t addr, uint16_t data)
{
    std::cout << "DiskController::process_bus_message" << std::endl;
    std::cout << "\t" << static_cast<std::underlying_type<BusMessage>::type>(t) << std::endl;
    printf("\taddr: %07o\n", addr);
    printf("\tdata: %07o\n", data);
    switch (t) {
        case BusMessage::DATI:
            if (addr == RXCS) {
                send(BusMessage::SSYN, addr, m_RXCS);
            } else if (addr == RXDB) {
                send(BusMessage::SSYN, addr, m_RXDB);
                clear_transfer_flag();
                clear_buffer_register();
            }
            break;

        case BusMessage::DATOB:
            if (addr == RXCS) {
                set_status_register(data);
                send(BusMessage::SSYN, addr, data);
            } else if (addr == RXDB) {
                m_RXDB = data;
                clear_transfer_flag();
                send(BusMessage::SSYN, addr, data);
            }
            break;

        case BusMessage::DATO:
            if (addr == RXCS) {
                set_status_register(data);
                send(BusMessage::SSYN, addr, data);
            } else if (addr == RXDB) {
                m_RXDB = data;
                clear_transfer_flag();
                send(BusMessage::SSYN, addr, data);
            }
            break;
        case BusMessage::DATIP: {
            break;
        }
        case BusMessage::MSYN: {
            break;
        }
        case BusMessage::SSYN: {
            break;
        }
        default: break;
    }

    printf("\tRXDB: %07o\n", m_RXDB);
    printf("\tRXCS: %07o\n", m_RXCS);
}

void DiskController::fill_buffer()
{
    if (m_buffer_index == SECTOR_SIZE) {
        return;
    }

    printf("\t%s(flags: %07o)\n", __FUNCTION__, m_RXCS);
    if (!is_transfer_flag_set()) {
        clear_transfer_flag();
        printf("\tfetching data, %d: %04o\n", m_buffer_index, m_RXDB);
        m_internal_buffer[m_buffer_index] = m_RXDB & 0377;
        clear_buffer_register();
        m_buffer_index++;
        if (m_buffer_index != SECTOR_SIZE) {
            set_transfer_flag();
        } else {
            clear_all_flags();
            set_done_flag();
            m_buffer_index = 0;
            m_state = DiskControllerState::DONE;
            m_function = DiskControllerFunction::IDLE;
        }
    } else {
        printf("\twaiting for write to register, %d\n", m_buffer_index);
    }
}

void DiskController::empty_buffer()
{
    if (m_buffer_index == SECTOR_SIZE) {
        return;
    }

    if (!is_transfer_flag_set()) {
        m_RXDB = m_internal_buffer[m_buffer_index];
        m_buffer_index++;
        if (m_buffer_index != SECTOR_SIZE) {
            set_transfer_flag();
        } else {
            clear_all_flags()
            set_done_flag();
            m_buffer_index = 0;
            m_state = DiskControllerState::DONE;
            m_function = DiskControllerFunction::IDLE;
        }
    }
}

void DiskController::read_sector()
{
    if (m_state == DiskControllerState::SECTOR && !is_transfer_flag_set()) {
        m_sector = m_RXDB;
        m_state = DiskControllerState::TRACK;
        set_transfer_flag();
        printf("\t%s(SECTOR, sector: %04o, track: %04o)\n",
            __FUNCTION__,
            m_sector,
            m_track
        );
    } else if (m_state == DiskControllerState::TRACK && !is_transfer_flag_set()) {
        m_track = m_RXDB;
        m_state = DiskControllerState::WRITE_SECTOR;
        printf("\t%s(TRACK, sector: %04o, track: %04o)\n",
            __FUNCTION__,
            m_sector,
            m_track
        );
    } else if (m_state == DiskControllerState::WRITE_SECTOR) {
        printf("\t%s(WRITE, sector: %04o, track: %04o)\n",
            __FUNCTION__,
            m_sector,
            m_track
        );

        if (m_disk_media != NULL) {
            // 3328 bytes per track.
            uint32_t track_offset = 3328 * m_track;
            uint32_t disk_offset = track_offset + (128 * m_sector);
            fseek(m_disk_media, disk_offset, SEEK_SET);
            fread(m_internal_buffer, sizeof(uint8_t), 128, m_disk_media);
        }

        m_state = DiskControllerState::DONE;
        m_function = DiskControllerFunction::IDLE;
        m_sector = 0;
        m_track = 0;
    }
}

void DiskController::write_sector()
{
    if (m_state == DiskControllerState::SECTOR && !is_transfer_flag_set()) {
        m_sector = m_RXDB;
        m_state = DiskControllerState::TRACK;
        set_transfer_flag();
        printf("\t%s(SECTOR, sector: %04o, track: %04o)\n",
            __FUNCTION__,
            m_sector,
            m_track
        );
    } else if (m_state == DiskControllerState::TRACK && !is_transfer_flag_set()) {
        m_track = m_RXDB;
        m_state = DiskControllerState::WRITE_SECTOR;
        printf("\t%s(TRACK, sector: %04o, track: %04o)\n",
            __FUNCTION__,
            m_sector,
            m_track
        );
    } else if (m_state == DiskControllerState::WRITE_SECTOR) {
        printf("\t%s(WRITE, sector: %04o, track: %04o)\n",
            __FUNCTION__,
            m_sector,
            m_track
        );

        if (m_disk_media != NULL) {
            // 3328 bytes per track.
            uint32_t track_offset = 3328 * m_track;
            uint32_t disk_offset = track_offset + (128 * m_sector);
            fseek(m_disk_media, disk_offset, SEEK_SET);
            fwrite(m_internal_buffer, sizeof(uint8_t), 128, m_disk_media);
        }

        m_state = DiskControllerState::DONE;
        m_function = DiskControllerFunction::IDLE;
        m_sector = 0;
        m_track = 0;
    }
}

void DiskController::clear_buffer_register()
{
    m_RXDB = 0;
}

void DiskController::set_status_register(uint16_t data)
{
    // Only allow writeable fields to be set.
    data &= RXCS_WRITE_MASK;
    m_RXCS |= data;
}

void DiskController::set_transfer_flag()
{
    if ((m_RXCS & static_cast<uint16_t>(RXCSFlag::XFER)) == 0) {
        m_RXCS |= static_cast<uint16_t>(RXCSFlag::XFER);
    }
}

void DiskController::clear_transfer_flag()
{
    m_RXCS = m_RXCS & ~static_cast<uint16_t>(RXCSFlag::XFER);
}

bool DiskController::is_transfer_flag_set()
{
    if (m_RXCS & static_cast<uint16_t>(RXCSFlag::XFER)) {
        return true;
    } else {
        return false;
    }
}

void DiskController::set_done_flag()
{
    if ((m_RXCS & static_cast<uint16_t>(RXCSFlag::DONE)) == 0) {
        m_RXCS |= static_cast<uint16_t>(RXCSFlag::DONE);
    }
}

void DiskController::clear_done_flag()
{
    if (m_RXCS & static_cast<uint16_t>(RXCSFlag::DONE)) {
        m_RXCS ^= static_cast<uint16_t>(RXCSFlag::DONE);
    }
}

bool DiskController::is_done_flag_set()
{
    if (m_RXCS & static_cast<uint16_t>(RXCSFlag::DONE)) {
        return true;
    } else {
        return false;
    }
}

void DiskController::clear_writeable_flags()
{
    m_RXCS = m_RXCS & ~RXCS_WRITE_MASK;
}

void DiskController::clear_all_flags()
{
    m_RXCS = 0;
}