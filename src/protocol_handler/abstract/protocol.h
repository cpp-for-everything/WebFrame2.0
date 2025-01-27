/**
 * @file protocol.h
 * @author Alex Tsvetanov (alex_tsvetanov_2002@abv.bg)
 * @brief Defines basic interfaces for the protocol and their parsers to follow.
 * @version 0.1
 * @date 2025-01-24
 *
 * @copyright Copyright (c) 2025
 *
 */
#include <memory>
#include <communication/request.h>
#include <communication/response.h>
#ifndef PROTOCOL_PRASER_FACTORY_H
#define PROTOCOL_PRASER_FACTORY_H

namespace protocol
{
	namespace abstract
	{
		/**
		 * @brief Interface for the protocols which would act as factories for their respective request parsers
		 *
		 */
		class Protocol
		{
		public:
			/**
			 * @brief Interface for the request parser of the specific protocol
			 *
			 */
			class ProtocolParser;

		public:
			/**
			 * @brief Construct a new defaul Protocol object
			 *
			 */
			Protocol() = default;

			/**
			 * @brief Destroy the Protocol object
			 *
			 */
			virtual ~Protocol() = default;

			/**
			 * @brief Creates a praser object for this request type
			 *
			 * @return std::shared_ptr<ProtocolParser> Pointer to newly created instance of a request parser for the
			 * specific protocol
			 */
			virtual std::shared_ptr<ProtocolParser> get_praser() = 0;

			/**
			 * @brief Send a response to a specific client
			 *
			 * @param client socket for communication with the client
			 * @param flags type of the socket
			 * @param request_data all of the data about the current request and its response
			 */
			virtual void send_data(SOCKET client, size_t flags, std::shared_ptr<ProtocolParser> request_data) = 0;
		};

		class Protocol::ProtocolParser
		{
		public:
			/**
			 * @brief Enum of the different states of the parser
			 *
			 */
			enum class ParsingState
			{
				NOT_STARTED = -2, /**< Parser waits for data to arrive to start processing */
				PROCESSING = -1,  /** < Parser cannot confirm or deny if the client is using the current protocol just
				                    yet */
				VERIFICATION_SUCCEEDED = 0, /** < Parser confirmed that the client uses the current protocol */
				VERIFICATION_FAILED = 1, /** < Parser identified ill-formed data which usually means that they are not
				                            using the current protocol */
				PARSING_DONE = 2 /** < Parser confirmed that the client uses the current protocol and all the data from
				                    the client has been put into the Request object */
			};

		protected:
			/**
			 * @brief Object containing the already parsed data from the request
			 *
			 */
			std::shared_ptr<Request> request;

			/**
			 * @brief Corresponding response object to the parsed request
			 *
			 */
			std::shared_ptr<Response> response;

			/**
			 * @brief Parser's current state
			 *
			 */
			ParsingState state;

			/**
			 * @brief Mutex for the changes on \ref ProtocolParser::state "state"
			 *
			 */
			std::mutex state_update;

			/**
			 * @brief Condition variable to notify for the changes of \ref ProtocolParser::state "state"
			 *
			 */
			std::condition_variable state_changed;

			/**
			 * @brief Initializes the data for the new request
			 *
			 */
			ProtocolParser()
			    : request(std::make_shared<Request>()), response(std::make_shared<Response>()),
			      state(ParsingState::NOT_STARTED)
			{
			}

			/**
			 * @brief Destroy the ProtocolParser object
			 *
			 */
			virtual ~ProtocolParser() = default;

			/**
			 * @brief Changes the current stage value to a new one
			 *
			 * @param new_state value of the new stage of the parsing
			 */
			void update_state(ParsingState new_state)
			{
				std::lock_guard lk(state_update);
				state = new_state;
			}

		public:
			/**
			 * @brief Get the state value
			 *
			 * @return ParsingState the value of parser's current state
			 */
			virtual ParsingState get_state() final
			{
				std::lock_guard lk(state_update);
				return state;
			}

			/**
			 * @brief Continues the processing of the request using the incoming chunk
			 *
			 * @param chunk the incoming chunk
			 */
			virtual void process_chunk(std::string_view chunk) = 0;
		};

	}  // namespace abstract
}  // namespace protocol

#endif  // PROTOCOL_PRASER_FACTORY_H