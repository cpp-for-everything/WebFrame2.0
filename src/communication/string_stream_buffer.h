#ifndef STRING_STREAM_BUFFER_H
#define STRING_STREAM_BUFFER_H

#include <fstream>
#include <string>
#include <string_view>
#include <mutex>
#include <stdexcept>
#include <cstring>

#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#endif

class TemporaryFileStringStream
{
public:
	TemporaryFileStringStream() : read_offset_(0), file_size_(0), last_chunk_size_(0)
	{
#if defined(_WIN32)
		char temp_path[MAX_PATH];
		char temp_file[MAX_PATH];

		// Get the temp directory path
		if (!GetTempPathA(MAX_PATH, temp_path))
		{
			throw std::runtime_error("Failed to get temp path");
		}

		// Generate a temp file name
		if (!GetTempFileNameA(temp_path, "TMP", 0, temp_file))
		{
			throw std::runtime_error("Failed to create temp file name");
		}

		filename_ = temp_file;

		// Open the temp file
		file_handle_ = CreateFile(filename_.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
		                          FILE_ATTRIBUTE_NORMAL, nullptr);
		if (file_handle_ == INVALID_HANDLE_VALUE)
		{
			throw std::runtime_error("Failed to open temp file");
		}
		mapping_handle_ = nullptr;
		mapped_data_ = nullptr;

		SYSTEM_INFO sys_info;
		GetSystemInfo(&sys_info);
		allocation_granularity_ = sys_info.dwAllocationGranularity;

#else
		allocation_granularity_ = sysconf(_SC_PAGESIZE);
		if (allocation_granularity_ <= 0)
		{
			throw std::runtime_error("Failed to retrieve allocation granularity");
		}
		// Generate a temp file and open it
		char temp_filename[] = "/tmp/tmpfileXXXXXX";
		fd_ = mkstemp(temp_filename);
		if (fd_ == -1)
		{
			throw std::runtime_error("Failed to create temp file");
		}

		filename_ = temp_filename;
		unlink(temp_filename);  // Ensure the file is removed when closed

		// Get the system page size
		page_size_ = sysconf(_SC_PAGESIZE);
		if (page_size_ <= 0)
		{
			throw std::runtime_error("Failed to retrieve page size");
		}
#endif
		update_file_size();
	}

	~TemporaryFileStringStream()
	{
#if defined(_WIN32)
		if (mapped_data_)
		{
			UnmapViewOfFile(mapped_data_);
		}
		if (mapping_handle_)
		{
			CloseHandle(mapping_handle_);
		}
		if (file_handle_ != INVALID_HANDLE_VALUE)
		{
			CloseHandle(file_handle_);
		}
		// Delete the temporary file explicitly (if needed)
		DeleteFileA(filename_.c_str());
#else
		if (mapped_data_)
		{
			munmap(mapped_data_, last_chunk_size_);
		}
		close(fd_);
#endif
	}

	void write(std::string_view data)
	{
		std::lock_guard<std::mutex> lock(write_mutex_);
#if defined(_WIN32)
		SetFilePointer(file_handle_, 0, nullptr, FILE_END);  // Move to the end of the file
		DWORD written;
		if (!WriteFile(file_handle_, data.data(), static_cast<DWORD>(data.size()), &written, nullptr))
		{
			throw std::runtime_error("Failed to write to file");
		}
#else
		lseek(fd_, 0, SEEK_END);  // Move to the end of the file
		if (::write(fd_, data.c_str(), data.size()) == -1)
		{
			throw std::runtime_error("Failed to write to file");
		}
#endif
		update_file_size();
	}

	std::string_view read_next_chunk(size_t chunk_size)
	{
		std::lock_guard<std::mutex> lock(read_mutex_);

		if (read_offset_ >= file_size_)
		{
			return {};  // No more data to read
		}

		size_t map_size = (std::min)(chunk_size, file_size_ - read_offset_);

		size_t aligned_offset = (read_offset_ / allocation_granularity_) * allocation_granularity_;
		size_t alignment_diff = read_offset_ - aligned_offset;

#if defined(_WIN32)

		if (!mapping_handle_ || mapped_data_ == nullptr || map_size != last_chunk_size_)
		{
			if (mapped_data_)
			{
				UnmapViewOfFile(mapped_data_);
			}
			if (mapping_handle_)
			{
				CloseHandle(mapping_handle_);
			}
			mapping_handle_ = CreateFileMapping(file_handle_, nullptr, PAGE_READONLY, 0, 0, nullptr);
			if (!mapping_handle_)
			{
				throw std::runtime_error("Failed to create file mapping");
			}
			mapped_data_ = static_cast<char*>(
			    MapViewOfFile(mapping_handle_, FILE_MAP_READ, static_cast<DWORD>(aligned_offset >> 32),
			                  static_cast<DWORD>(aligned_offset & 0xFFFFFFFF), map_size + alignment_diff));
			if (!mapped_data_)
			{
				throw std::runtime_error("Failed to map view of file");
			}
		}

		std::string_view chunk(mapped_data_ + alignment_diff, map_size);
		read_offset_ += map_size;
		last_chunk_size_ = map_size;

		return chunk;

#else
		// Unmap the previous mapping if necessary
		if (mapped_data_)
		{
			munmap(mapped_data_, last_chunk_size_);
		}

		// Map the file
		mapped_data_ = static_cast<char*>(mmap(nullptr, map_size, PROT_READ, MAP_SHARED, fd_, aligned_offset));
		if (mapped_data_ == MAP_FAILED)
		{
			perror("mmap failed");
			throw std::runtime_error("Failed to map file");
		}

		// Update offsets and sizes
		std::string_view chunk(mapped_data_ + alignment_diff, chunk_size);
		read_offset_ += chunk_size;
		last_chunk_size_ = map_size;

		return chunk;
#endif
	}

private:
	void update_file_size()
	{
#if defined(_WIN32)
		LARGE_INTEGER size;
		if (!GetFileSizeEx(file_handle_, &size))
		{
			throw std::runtime_error("Failed to get file size");
		}
		file_size_ = static_cast<size_t>(size.QuadPart);
#else
		file_size_ = lseek(fd_, 0, SEEK_END);
#endif
	}

	std::string filename_;
	size_t page_size_;
	size_t read_offset_;
	size_t file_size_;
	size_t last_chunk_size_;
	size_t allocation_granularity_;
	std::mutex write_mutex_;
	std::mutex read_mutex_;

#if defined(_WIN32)
	HANDLE file_handle_;
	HANDLE mapping_handle_;
	char* mapped_data_;
#else
	int fd_;
	char* mapped_data_;
#endif
};

#endif  // STRING_STREAM_BUFFER_H
